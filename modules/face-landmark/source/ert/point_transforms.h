// Copyright (C) 2003  Davis E. King (davis@dlib.net)
// License: Boost Software License   See LICENSE.txt for the full license.
#ifndef DLIB_POINT_TrANSFORMS_H_
#define DLIB_POINT_TrANSFORMS_H_

#include "point_transforms_abstract.h"
#include <vector>
#include "Serializable.hh"

cv::Point2f operator*(const cv::Mat &M, const cv::Point2f& p);

namespace dlib
{


// ----------------------------------------------------------------------------------------

	Point2f operator*(const cv::Mat &M, const cv::Point2f& p)
	{
		cv::Mat src(2/*rows*/,1 /* cols */,CV_64F);

		src.at<double>(0,0)=p.x;
		src.at<double>(1,0)=p.y;
		//src.at<float>(2,0)=1.0;

		cv::Mat dst = M*src; //USE MATRIX ALGEBRA
		return cv::Point2f(dst.at<double>(0,0),dst.at<double>(1,0));
	}

	Point2f operator+( const cv::Point2f& p1, const cv::Point2f& p2 )
	{
		return Point2f( p1.x + p2.x, p1.y + p2.y );
	}

    class point_transform_affine
    {
    public:

        point_transform_affine (
        )
        {
            //m = identity_matrix<double>(2);
            b.x = 0;
            b.y = 0;
        }

        point_transform_affine (
            const Mat& m_,
            const Point2f& b_
        ) :m(m_), b(b_)
        {
        }

        point_transform_affine (
            const Mat& m_,
            const Mat& b_
        ) :m(m_)
        {
			b = Point2f( b_.at<double>(0,0), b_.at<double>(1,0) );
			/*std::cout << "m_ = " << m_ << std::endl;
			std::cout << "b_ = " << b_ << std::endl;
			std::cout << "b = " << b << std::endl;*/
        }

        const Point2f operator() (
            const Point2f& p
        ) const
        {
            return m*p + b;
        }

        const Mat& get_m(
        ) const { return m; }

        const Point2f& get_b(
        ) const { return b; }

        inline friend void serialize (const point_transform_affine& item, std::ostream& out)
        {
            /*serialize(item.m, out);
            serialize(item.b, out);*/
        }

        inline friend void deserialize (point_transform_affine& item, std::istream& in)
        {
            /*deserialize(item.m, in);
            deserialize(item.b, in);*/
        }

    private:
        Mat m;
        Point2f b;
    };

// ----------------------------------------------------------------------------------------




    inline point_transform_affine operator* (
        const point_transform_affine& lhs,
        const point_transform_affine& rhs
    )
    {
        return point_transform_affine( lhs.get_m() * rhs.get_m(),
			lhs.get_m() * rhs.get_b() + lhs.get_b() );
    }


    //template <typename T>
    point_transform_affine find_affine_transform (
        const std::vector<Point2f>& from_points,
        const std::vector<Point2f>& to_points
    )
    {
        // make sure requires clause is not broken
		assert(from_points.size() == to_points.size() &&
			from_points.size() >= 3);
        /*DLIB_ASSERT(from_points.size() == to_points.size() &&
                    from_points.size() >= 3,
            "\t point_transform_affine find_affine_transform(from_points, to_points)"
            << "\n\t Invalid inputs were given to this function."
            << "\n\t from_points.size(): " << from_points.size()
            << "\n\t to_points.size():   " << to_points.size()
            );*/

        Mat P(3, from_points.size(), CV_64F);
        Mat Q(2, from_points.size(), CV_64F);

        for (unsigned long i = 0; i < from_points.size(); ++i)
        {
            P.at<double>(0,i) = from_points[i].x;
            P.at<double>(1,i) = from_points[i].y;
            P.at<double>(2,i) = 1;

            Q.at<double>(0,i) = to_points[i].x;
            Q.at<double>(1,i) = to_points[i].y;
        }

//std::cout << "P = " << P << std::endl;
//std::cout << "Q = " << Q << std::endl;

//std::cout << "P.inv() = " << P.inv(DECOMP_SVD) << std::endl;

        Mat m = Q * P.inv(DECOMP_SVD);
//std::cout << "m = " << m << std::endl;
        //return point_transform_affine(subm(m,0,0,2,2), colm(m,2));
        return point_transform_affine(
			m( Range(0, 2), Range(0, 2) ),
			m( Range(0, m.rows), Range(2,3) ) );
    }

// ----------------------------------------------------------------------------------------


	Point2f operator/( const Point2f &p1, size_t i )
	{
		return Point2f( p1.x / i, p1.y / i );
	}


	double length_squared( const Point2f &p )
	{
		return (double)p.x * (double)p.x + (double)p.y * (double)p.y;
	}

	/**
	 * Compute the transform affine matrix that transform the 'from' points to the 'to' points.
	 */
    point_transform_affine find_similarity_transform (
        const std::vector<Point2f>& from_points,
        const std::vector<Point2f>& to_points
    )
    {
        // make sure requires clause is not broken
        /*DLIB_ASSERT(from_points.size() == to_points.size() &&
                    from_points.size() >= 2,
            "\t point_transform_affine find_similarity_transform(from_points, to_points)"
            << "\n\t Invalid inputs were given to this function."
            << "\n\t from_points.size(): " << from_points.size()
            << "\n\t to_points.size():   " << to_points.size()
            );*/

        // We use the formulas from the paper: "Least-squares estimation of transformation
        // parameters between two point patterns" by Umeyama.  They are equations 34 through
        // 43.

        Point2f mean_from, mean_to;
        double sigma_from = 0, sigma_to = 0;
        Mat cov;

		// compute the mean (eq. 34 and 35)
        for (unsigned long i = 0; i < from_points.size(); ++i)
        {
            mean_from += from_points[i];
            mean_to += to_points[i];
        }
        mean_from = mean_from / from_points.size();
        mean_to   = mean_to / from_points.size();

		// compute the variance and covariance (eq. 36, 37 and 38)
        for (unsigned long i = 0; i < from_points.size(); ++i)
        {
            sigma_from += length_squared(from_points[i] - mean_from);
            sigma_to += length_squared(to_points[i] - mean_to);
            if (cov.rows == 0)
				cov = Mat(to_points[i] - mean_to)* Mat(from_points[i] - mean_from).t();
			else
				cov += Mat(to_points[i] - mean_to)* Mat(from_points[i] - mean_from).t();
        }
        sigma_from /= from_points.size();
        sigma_to   /= from_points.size();
        cov        /= from_points.size();
/*std::cout << "mean_from = " << mean_from << std::endl;
std::cout << "mean_to = " << mean_to << std::endl;
std::cout << "cov = " << cov << std::endl;*/
		Mat u, v, s, d;

		cv::SVD svd;
		svd.compute(cov, d, u, v);
		d.convertTo(d, CV_64F);
		u.convertTo(u, CV_64F);
		v.convertTo(v, CV_64F);
		//u = u.t();
		//v = v.t();
		// adjust the matrix 'd' to be used above
		cv::Mat temp = cv::Mat::zeros(2, 2, CV_64F);
		temp.at<double>(1,1) = d.at<double>(0,1);
		temp.at<double>(0,0) = d.at<double>(0,0);
		d = temp;

		u( cv::Range::all(), cv::Range(1,2) ) *= -1;
		v( cv::Range(1,2), cv::Range::all() ) *= -1;

        //svd(cov, u,d,v);
/*std::cout << "u = " << u << std::endl;
std::cout << "d = " << d << std::endl;
std::cout << "v = " << v << std::endl << std::endl;*/
        //s = identity_matrix(cov);
        s = cv::Mat::eye( cov.size(), CV_64F );

        if (cv::determinant(cov) < 0 || (cv::determinant(cov) == 0 && cv::determinant(u)*cv::determinant(v)<0))
        {
            if (d.at<double>(1,1) < d.at<double>(0,0))
                s.at<double>(1,1) = -1;
            else
                s.at<double>(0,0) = -1;
        }

//std::cout << "trace(d*s) = " << cv::sum((d*s).diag())[0] << std::endl;

        Mat r = (u*s)*v.t();
//std::cout << "r = " << r << std::endl;
//std::cout << "s = " << s << std::endl;
        double c = 1;
        if (sigma_from != 0)
        {
			c = cv::sum((d*s).diag())[0];
            c = 1.0 / sigma_from * c;
		}
        Point2f t = mean_to - c*r*mean_from;
//std::cout << "c = " << c << std::endl;
        return point_transform_affine(c*r, t);
    }
}

#endif // DLIB_POINT_TrANSFORMS_H_

