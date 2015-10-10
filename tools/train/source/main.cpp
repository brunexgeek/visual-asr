// The contents of this file are in the public domain. See LICENSE_FOR_EXAMPLE_PROGRAMS.txt
/*

    This example program shows how to use dlib's implementation of the paper:
        One Millisecond Face Alignment with an Ensemble of Regression Trees by
        Vahid Kazemi and Josephine Sullivan, CVPR 2014

    In particular, we will train a face landmarking model based on a small dataset
    and then evaluate it.  If you want to visualize the output of the trained
    model on some images then you can run the face_landmark_detection_ex.cpp
    example program with sp.dat as the input model.

    It should also be noted that this kind of model, while often used for face
    landmarking, is quite general and can be used for a variety of shape
    prediction tasks.  But here we demonstrate it only on a simple face
    landmarking task.
*/


#include <opencv2/opencv.hpp>
//#include "../../../modules/face-landmark/source/ert/ShapePredictor.hh"
#include "../../../modules/face-landmark/source/ert/ShapePredictorTrainer.hh"

#include <iostream>
#include <fstream>
#include <cstring>

using namespace ert;
using namespace std;



std::vector<std::vector<double> > get_interocular_distances (
    const std::vector<std::vector<ObjectDetection*> >& objects
);
/*!
    ensures
        - returns an object D such that:
            - D[i][j] == the distance, in pixels, between the eyes for the face represented
              by objects[i][j].
!*/



void main_loadData(
	const std::string &script,
	std::vector<Mat*> &images,
	std::vector<std::vector<ObjectDetection*> > &annots  )
{
	char line[128];
	int imageCount = 0;

	ifstream list(script.c_str());

	// read the amount of files
	list.getline(line, sizeof(line));
	imageCount = atoi(line);

	images.resize(imageCount);
	annots.resize(imageCount);

	for (int i = 0; i < imageCount; ++i)
	{
		// load the image
		list.getline(line, sizeof(line) - 5);
		std::cout << "Loading " << line << std::endl;
#if (0)
		cv::Mat current;
		cv::cvtColor(imread(line), current, CV_BGR2GRAY);
#else
		cv::Mat temp[3];
		split(imread(line), temp);
		temp[0].convertTo(temp[0], CV_32F);
		temp[1].convertTo(temp[1], CV_32F);
		temp[2].convertTo(temp[2], CV_32F);
		cv::Mat current = temp[0] + temp[1] + temp[2];
		current /= 3.0;
		current.convertTo(current, CV_8U);
		/*for (int r = 0; r < current.rows; ++r)
			for (int c = 0; c < current.cols; ++c)
				current.at<float>(r, c) = std::ceil( current.at<float>(r, c) );*/
		//current.convertTo(current, CV_8U);
//std::getchar();
/*std::cout << temp[0]( cv::Range(0, 5), cv::Range(0, 5) ) << std::endl
		  << temp[1]( cv::Range(0, 5), cv::Range(0, 5) ) << std::endl
		  << temp[2]( cv::Range(0, 5), cv::Range(0, 5) ) << std::endl
		  << current( cv::Range(0, 5), cv::Range(0, 5) ) << std::endl;
*/
/*cv::imshow("Test", temp[0]);
cv::waitKey(0);*/
#endif
		images[i] = new cv::Mat(current);
		// load the annotations
		int pos = strrchr(line, '.') - line;
		if (pos >= 0)
		{
			line[pos] = 0;
			strcat(line, ".pts");
		}
		else
			return;
		std::vector<ObjectDetection*> annot;
		annot.push_back( new ObjectDetection(line) );
		annots[i] = annot;
	}
}



int main(int argc, char** argv)
{
	/*cv::Mat cov = cv::Mat(2, 2, CV_64F);
	cov.at<double>(0,0) = 0.0411864;
	cov.at<double>(0,1) = -0.00438393;
	cov.at<double>(1,0) = 0.00140244;
	cov.at<double>(1,1) = 0.0521182;

	cv::Mat d, u, v;
	cv::SVD svd;
	svd.compute(cov, d, u, v, cv::SVD::FULL_UV);

	Mat _u = cv::Mat::zeros(u.rows, u.cols, u.type());
	u.row(1).copyTo( _u.row(0) );
	u.row(0).copyTo( _u.row(1) );
	_u.row(1) *= -1;

	Mat _v = cv::Mat::zeros(v.rows, v.cols, v.type());
	v.row(1).copyTo( _v.row(0) );
	v.row(0).copyTo( _v.row(1) );
	_v.col(0) *= -1;

	std::cout << "cov = " << cov << std::endl;
	std::cout << "u = " << _u << std::endl;
	std::cout << "d = " << d << std::endl;
	std::cout << "v = " << _v << std::endl << std::endl;

	return 0;*/

    try
    {
        // In this example we are going to train a shape_predictor based on the
        // small faces dataset in the examples/faces directory.  So the first
        // thing we do is load that dataset.  This means you need to supply the
        // path to this faces folder as a command line argument so we will know
        // where it is.
        if (argc != 2)
        {
            cout << "Give the path to the examples/faces directory as the argument to this" << endl;
            cout << "program.  For example, if you are in the examples folder then execute " << endl;
            cout << "this program by running: " << endl;
            cout << "   ./train_shape_predictor_ex faces" << endl;
            cout << endl;
            return 0;
        }
        const std::string script = argv[1];

		std::vector<Mat*> images_train, images_test;
		std::vector<std::vector<ObjectDetection*> > annots_train, annots_test;
		main_loadData(script, images_train, annots_train);

        // Now we load the data.  These XML files list the images in each
        // dataset and also contain the positions of the face boxes and
        // landmarks (called parts in the XML file).  Obviously you can use any
        // kind of input format you like so long as you store the data into
        // images_train and faces_train.  But for convenience dlib comes with
        // tools for creating and loading XML image dataset files.  Here you see
        // how to load the data.  To create the XML files you can use the imglab
        // tool which can be found in the tools/imglab folder.  It is a simple
        // graphical tool for labeling objects in images.  To see how to use it
        // read the tools/imglab/README.txt file.
        //load_image_dataset(images_train, faces_train, faces_directory+"/training_with_face_landmarks.xml");
        //load_image_dataset(images_test, faces_test, faces_directory+"/testing_with_face_landmarks.xml");

        // Now make the object responsible for training the model.
        ShapePredictorTrainer trainer;
        // This algorithm has a bunch of parameters you can mess with.  The
        // documentation for the shape_predictor_trainer explains all of them.
        // You should also read Kazemi's paper which explains all the parameters
        // in great detail.  However, here I'm just setting three of them
        // differently than their default values.  I'm doing this because we
        // have a very small dataset.  In particular, setting the oversampling
        // to a high amount (300) effectively boosts the training set size, so
        // that helps this example.
        trainer.set_oversampling_amount(300);
		trainer.set_cascade_depth(10);
        trainer.set_num_trees_per_cascade_level(500);
        // I'm also reducing the capacity of the model by explicitly increasing
        // the regularization (making nu smaller) and by using trees with
        // smaller depths.
        trainer.set_nu(0.05);
        trainer.set_tree_depth(2);


        // Tell the trainer to print status messages to the console so we can
        // see how long the training will take.
        trainer.be_verbose();

        // Now finally generate the shape model
        ShapePredictor sp = trainer.train( images_train, annots_train);
        
        std::ofstream output("./model.dat");
        sp.serialize(output);
        output.close();

		ShapePredictor model;
		std::ifstream input("./model.dat");
		model.deserialize(input);
		input.close();


        // Now that we have a model we can test it.  This function measures the
        // average distance between a face landmark output by the
        // shape_predictor and where it should be according to the truth data.
        // Note that there is an optional 4th argument that lets us rescale the
        // distances.  Here we are causing the output to scale each face's
        // distances by the interocular distance, as is customary when
        // evaluating face landmarking systems.
        cout << "mean training error: "<<
            test_shape_predictor(model, images_train, annots_train, get_interocular_distances(annots_train)) << endl;

        // The real test is to see how well it does on data it wasn't trained
        // on.  We trained it on a very small dataset so the accuracy is not
        // extremely high, but it's still doing quite good.  Moreover, if you
        // train it on one of the large face landmarking datasets you will
        // obtain state-of-the-art results, as shown in the Kazemi paper.
        //cout << "mean testing error:  "<<
        //    test_shape_predictor(sp, images_train, annots_train, get_interocular_distances(annots_train)) << endl;


		/*char key;
		do { key = cv::waitKey(0); } while (key != 0x27);*/

        // Finally, we save the model to disk so we can use it later.
        //serialize("sp.dat") << sp;
    }
    catch (exception& e)
    {
        cout << "\nexception thrown!" << endl;
        cout << e.what() << endl;
    }
}

// ----------------------------------------------------------------------------------------

double interocular_distance (
    const ObjectDetection& det
)
{
    Point2f l, r;
    double cnt = 0;
    // Find the center of the left eye by averaging the points around
    // the eye.
    for (unsigned long i = 36; i <= 41; ++i)
    {
        l += det.part(i);
        ++cnt;
    }
    l = l / cnt;

    // Find the center of the right eye by averaging the points around
    // the eye.
    cnt = 0;
    for (unsigned long i = 42; i <= 47; ++i)
    {
        r += det.part(i);
        ++cnt;
    }
    r = r / cnt;

    // Now return the distance between the centers of the eyes
std::cout << cv::norm(l-r) << std::endl;
    return cv::norm(l-r);
}

std::vector<std::vector<double> > get_interocular_distances (
    const std::vector<std::vector<ert::ObjectDetection*> >& objects
)
{
    std::vector<std::vector<double> > temp(objects.size());
    for (unsigned long i = 0; i < objects.size(); ++i)
    {
        for (unsigned long j = 0; j < objects[i].size(); ++j)
        {
            temp[i].push_back(interocular_distance(*objects[i][j]));
        }
    }
    return temp;
}

// ----------------------------------------------------------------------------------------
