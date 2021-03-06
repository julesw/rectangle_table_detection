/**
 * Data structures and functions in relation to the geometry of a table.
 * \file tableDetectionGeometricModel.h
 *
 * These function mainly work over Eigen and are used to compute a model of the table
 **/
#ifndef _GEOMETRY_UTILITIES_H_
#define _GEOMETRY_UTILITIES_H_

#include <cmath>
#include <iostream>
#include <ostream>
#include <fstream>

#include <ros/ros.h>
#include <tf/tf.h>
#include <tf/transform_listener.h>

// PCL specific includes
#include <pcl/pcl_base.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/ModelCoefficients.h>

#include "UniqueLoopsMngr.h"

//#define _DEBUG_FUNCTIONS_


//a debug utility
#define _PRINT_HERE_(s) ROS_DEBUG_STREAM("in " << __FILE__ << " line " << __LINE__ << "\t-\t" << s)


/**
 * A class for handling all geometric operations on the model.
 * This class manages the geometric model of the table we wantto detect
 */
class tableDetectionGeometricModel {

	public:

		struct Line_def;

		/**
		 * a vertex representation with a list of lines passing through this point.
		 * This structure represent a node of a graph formed by vertices and lines
		 * \see recursively_find_connected_vertices()
		 **/
		struct Vertex_def{
			std::vector<Line_def*> edges; /**< edges passing by this point (usually, the vertex is defined as the intersection of these edges)**/
			Eigen::Vector3f vertex; /**< the position in space of this vertex**/
		};

		/**
		 * a line representation
		 * \see Vertex_def
		 * \see recursively_find_connected_vertices()
		 **/
		struct Line_def{
			Eigen::ParametrizedLine<float,3> line; /**< the line definition as an Eigen object**/
			std::vector<Vertex_def*> vertices; /**< the vertices that are on this line**/
			bool marked; /**< a marker for the exploration of the graph**/
		};


		/**
		 * defines an oriented rectangle in a 3D space.
		 * The vector \f$ \overrightarrow{x} x \overrightarrow{y} \f$ indicates the orientation of the rectangle.
		 * So this 3 vectors and the point would form an orthogonal repair in the space.
		 **/
		struct Rectangle{
			Eigen::Vector3f vect_x; /**< the vector defining the 1st edge**/
			Eigen::Vector3f vect_y; /**< the vector defining the 2nd edge, orthogonal to the 1st**/
			Eigen::Vector3f point; /**< a point that is a vertex of the rectangle**/
		};

		/**Full builder*/
		tableDetectionGeometricModel(Eigen::Vector3f origin, Eigen::Vector3f vertical, double param_cos_ortho_tolerance);
		/**Simple builder.
		 * \warning you must call setVerticalLine() before using methods of this instance.
		 */
		tableDetectionGeometricModel(double param_cos_ortho_tolerance=0.);
		virtual ~tableDetectionGeometricModel();
		
		/** Clear this object.
		 * Call this between each new modelisation to clear all stored vertices, borders and rectangles.
		 * Preserves vertical line and cos_ortho_tolerance, and private attributes used for model continuity.
		 */
		void clear();

	private:
		void init(double cos_ortho_tolerance);
			

	private:
		Eigen::ParametrizedLine<float, 3> _vertical_line;

		double _cos_ortho_tolerance;

		boost::shared_ptr<Rectangle> _previous_best_rectangle;
		UniqueLoopsMngr<Vertex_def*> _verticesLoopsMngr;

	public:
		std::vector<Line_def*> borders_;
		std::vector<Vertex_def*> vertices_;
		std::vector<boost::shared_ptr<Rectangle> > possible_rectangles_;

	public:
		/**test if the borders indicated by i and j are orthogonals.
		 * Test is made with the tolerance indicated by the attribute
		 * \see setCosOrthoTolerance()
		 */
		bool areBordersOrthogonal(int i,int j);
		/** adds a vertex to the model from two edges of the model.
		 * \warning Does NOT checks if borders are orthogonal. You MUST do it before so the model is not corrupted.
		 */
		bool addVertexFromEdges(int i,int j);
		/// Add a border to the model.
		void addBorder(const Eigen::VectorXf& coeffs);

		///Constructs and add a rectangle to the list of detected rectangles.
		///\see compute_rectangle()
		bool addPossibleRectangle(const std::vector<int>& vertices_indices);
		bool addPossibleRectangle(const std::vector<Vertex_def*>::iterator& first_vertex, const std::vector<Vertex_def*>::iterator& last_vertex);
		bool addPossibleRectangle(const std::vector<Vertex_def*>& vertices);

		void setVerticalLine(const Eigen::Vector3f& origin, const Eigen::Vector3f& direction);
		/**Sets the vertical line to use as reference*/
		void setVerticalLine(const Eigen::ParametrizedLine<float,3>& vertical_line);
		/** returns the vertical line as set by the user*/
		Eigen::ParametrizedLine<float,3> getVerticalLine();
		/** get the origin of the vertical reference line set by the user.
		 * \warning vertical line must be specified
		 */
		Eigen::Vector3f getVerticalOrigin();
		/** get the direction of the vertical reference line set by the user.
		 * \warning vertical line must be specified
		 */
		Eigen::Vector3f getVerticalDirection();
		/**vertices in the model*/
		int verticesCount();
		/**borders in the model*/
		int bordersCount();
		/**possible rectangles in the model*/
		int possibleRectanglesCount();
		/** set the tolerance to use to determine if the cosinus of an angle is approx 0.*/
		void setCosOrthoTolerance(double tol);
		/** get the tolerance set by the user*/
		double getCosOrthoTolerance();

#ifdef _DEBUG_FUNCTIONS_
		//TODO remove debug function
		int print_graph(Line_def* edge, int index, std::ostream& outstream);
#endif

		/** A redefinition of the dot product.
		 * \param[in] v1 first vector
		 * \param[in] v2 2nd vector
		 * \return the dot product \f$ \overrightarrow{v_1} \cdot \overrightarrow{v_2} \f$
		 *
		 * Apparently, the Eigen dot product is not efficient
		 */
		inline float dot_product(const Eigen::Vector3f& v1, const Eigen::Vector3f& v2);

		/**
		 * recursively explore the vertices of an edge an add connected points to vertices.
		 * \param[out] vertices vector to which connected vertices will be appended
		 * \param[in,out] edge edge where exploration starts
		 *
		 * Run over the graph formed by Line_def and Vertex_def and returns a list of pointers to vertices
		 * that are all connected by edges. This list can represent one or more overlapping rectangles, assuming
		 * that a vertex is defined as the intersection of two orthogonals lines.
		 **/
		void recursively_find_connected_vertices(std::vector<Vertex_def*>& vertices, Line_def* edge);

	private:
		void recursively_find_rectangles_from(Vertex_def* vertex);
		void recursively_find_rectangles(std::vector<Vertex_def*>& vertices,
			 	const Line_def* prev_edge);

	public:
		/**
		 * runs over all vertices and tries to find all the possible rectangles.
		 *
		 */
		int find_all_possible_rectangles();

#ifdef _DEBUG_FUNCTIONS_
	public:
#else
	private:
#endif
		/**
		 * select the rectangle that best describes the plane in the point cloud pc_plan.
		 *
		 * \param[in] possible_rectangles an array of rectangles to compare.
		 * \param[in] pc_plan the point cloud containing the plane we want to represent.
		 * \param[in] indices the indices of the points in pc_plan that actually represent the plane.
		 * \param[in] required_score the minimum score(in proportion of matching points) required
		 * to be considered as a describing rectangle.
		 * \param[in] lead_score the difference of score between 1st and 2nd best rectangles so that
		 * there is no possible ambiguity between who is the best.
		 * \param[in] n_samples the number of points to try.
		 * \param[out] best_score the best score.
		 * \param[out] best_index the index of the best rectangle in the \c possible_rectangles array.
		 * \param[out] not_matched_points the indices of the point that were not matched at least once.
		 * \return the index of the selected rectangle, or -1 if no rectangle satisfies the required_score and
		 * the lead_score constraints.
		 *
		 *
		 * Select randomly n_samples points in the input pc_plan
		 * and check for each rectangle if it contains this point
		 * If the best rectangle have a score > required_score
		 * and a <lead_score> points lead on the 2nd best, 
		 * we return it's index.
		 *
		 * \note the last 3 parameters are for debug. Use overriding function select_best_matching_rectangle() for general use.
		 *
		 **/
		int select_best_matching_rectangle(pcl::PointCloud<pcl::PointXYZRGBA>::ConstPtr pc_plan, pcl::PointIndices::ConstPtr indices,float required_score,float lead_score, int n_samples, float& best_score,int& best_index,pcl::PointIndices::Ptr not_matched_points);

	public:
		/**
		 * select the rectangle that best describes the plane in the point cloud pc_plan.
		 *
		 * \param[in] pc_plan the point cloud containing the plane we want to represent.
		 * \param[in] indices the indices of the points in pc_plan that actually represent the plane.
		 * \param[in] required_score the minimum score(in proportion of matching points) required
		 * to be considered as a describing rectangle.
		 * \param[in] lead_score the difference of score between 1st and 2nd best rectangles so that
		 * there is no possible ambiguity between who is the best.
		 * \param[in] n_samples the number of points to try.
		 * \return the index of the selected rectangle, or -1 if no rectangle satisfies the required_score and
		 * the lead_score constraints.
		 *
		 *
		 * Select randomly n_samples points in the input pc_plan
		 * and check for each rectangle if it contains this point
		 * If the best rectangle have a score > required_score
		 * and a <lead_score> points lead on the 2nd best, 
		 * we return it's index.
		 *
		 **/
		int select_best_matching_rectangle(pcl::PointCloud<pcl::PointXYZRGBA>::ConstPtr pc_plan,
				pcl::PointIndices::ConstPtr indices,
				float required_score, float lead_score,
				int n_samples);

		bool point_is_in_rectangle(const pcl::PointXYZRGBA point, const boost::shared_ptr<Rectangle> rect, float relative_thresh=0.01);

		/**
		 * returns a rectangle from the given vertices.
		 * \param[in] vertices_indices the the indices of the vertices of the rectangle to compute.
		 * \return a shared_ptr to the rectangle, as defined bellow.
		 *
		 * The returned rectangle is such that all points in it can
		 * be defined as \f$ a \cdot \overrightarrow{x} + b \cdot \overrightarrow{y}\f$
		 * with \f$(a,b) \geq (0,0)\f$ \n
		 * \f$\overrightarrow{x}\f$ (vect_x) and \f$\overrightarrow{y}\f$ (vect_y) are choosen so that \f$\overrightarrow{x} \times \overrightarrow{y}\f$ have (approx) the same direction 
		 * than the given vertical
		 **/
		boost::shared_ptr<Rectangle> compute_rectangle(const std::vector<int>& vertices_indices);
		boost::shared_ptr<Rectangle> compute_rectangle(const std::vector<Vertex_def*>& vertices);
		boost::shared_ptr<Rectangle> compute_rectangle(std::vector<Vertex_def*>::const_iterator from_vertex, std::vector<Vertex_def*>::const_iterator to_vertex);

		/**
		 * Compute a transform so that the child frame have given x and y axis.
		 * \param[in] origin the origin of the child frame in the parent frame coordinates.
		 * \param[in] x_axis a vector indicating the desired x axis direction of the child frame.
		 * \param[in] y_axis idem.
		 * \return the transform, as described.
		 *
		 * \note axes CAN be not normalized
		 **/
		tf::Transform computeTransform(const Eigen::Vector3f& origin, const Eigen::Vector3f& x_axis,const Eigen::Vector3f& y_axis);

};
#endif // _GEOMETRY_UTILITIES_H_
