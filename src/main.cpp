#include <gtsam/geometry/Pose3.h>
#include <gtsam/geometry/Point3.h>
#include <gtsam/inference/Symbol.h>
#include <gtsam/nonlinear/ISAM2.h>

// iSAM2 requires as input a set set of new factors to be added stored in a factor graph,
// and initial guesses for any new variables used in the added factors
#include <gtsam/nonlinear/NonlinearFactorGraph.h>
#include <gtsam/nonlinear/Values.h>

#include <gtsam/slam/PriorFactor.h>
#include <gtsam/slam/BetweenFactor.h>

#include <iostream>
#include <cmath>
#include <cstring>
#include <opencv2/opencv.hpp>

#include "CLC.h"
#include <vector>

using namespace cv;
using namespace std;
using namespace Eigen;
using namespace gtsam;
#include <gtsam/nonlinear/NonlinearFactor.h>
/*
class CLCBinaryFactor: public NoiseModelFactor2<Pose3, Pose3> {

  // The factor will hold a measurement consisting of an (X,Y) location
  // We could this with a Point2 but here we just use two doubles
  Pose3 mx_, my_;

public:
  /// shorthand for a smart pointer to a factor
  typedef boost::shared_ptr<CLCBinaryFactor> shared_ptr;

  // The constructor requires the variable key, the (X, Y) measurement value, and the noise model
  CLCBinaryFactor(Key i, Key j, Pose3 x, Pose3 y, const SharedNoiseModel& model):
    NoiseModelFacto2<Pose3, Pose3>(model, j), mx_(x), my_(y) {}

  virtual ~CLCBinaryFactor() {}

  // Using the NoiseModelFactor1 base class there are two functions that must be overridden.
  // The first is the 'evaluateError' function. This function implements the desired measurement
  // function, returning a vector of errors when evaluated at the provided variable value. It
  // must also calculate the Jacobians for this measurement function, if requested.

  // h(x)-z -> between(z,h(x)) for Rot manifold
  Vector evaluateError(const Pose& pose, const Point& point,
      boost::optional<Matrix&> H1, boost::optional<Matrix&> H2) const {
      //H is Jacobian of error function
      // The measurement function for a GPS-like measurement is simple:
      // error_x = pose.x - measurement.x
      // error_y = pose.y - measurement.y
      // Consequently, the Jacobians are:
      // [ derror_x/dx  derror_x/dy  derror_wqwwx/dtheta ] = [1 0 0]
      // [ derror_y/dx  derror_y/dy  derror_y/dtheta ] = [0 1 0]
      //if (H) (*H) = (Matrix(2,3) << 1.0,0.0,0.0, 0.0,1.0,0.0);
      //return (Vector(2) << q.x() - mx_, q.y() - my_);
    Matrix H11, H21, H12, H22;
    boost::optional<Matrix&> H11_ = H1 ? boost::optional<Matrix&>(H11) : boost::optional<Matrix&>();
    boost::optional<Matrix&> H21_ = H1 ? boost::optional<Matrix&>(H21) : boost::optional<Matrix&>();
    boost::optional<Matrix&> H12_ = H2 ? boost::optional<Matrix&>(H12) : boost::optional<Matrix&>();
    boost::optional<Matrix&> H22_ = H2 ? boost::optional<Matrix&>(H22) : boost::optional<Matrix&>();

    Rot y1 = pose.bearing(point, H11_, H12_);
    Vector e1 = Rot::Logmap(measuredBearing_.between(y1));

    double y2 = pose.range(point, H21_, H22_);
    Vector e2 = (Vector(1) << y2 - measuredRange_);

    if (H1) *H1 = gtsam::stack(2, &H11, &H21);
    if (H2) *H2 = gtsam::stack(2, &H12, &H22);
    return concatVectors(2, &e1, &e2);
  }


  // The second is a 'clone' function that allows the factor to be copied. Under most
  // circumstances, the following code that employs the default copy constructor should
  // work fine.
  virtual gtsam::NonlinearFactor::shared_ptr clone() const {
    return boost::static_pointer_cast<gtsam::NonlinearFactor>(
        gtsam::NonlinearFactor::shared_ptr(new CLCBinaryFactor(*this))); }

  // Additionally, we encourage you the use of unit testing your custom factors,
  // (as all GTSAM factors are), in which you would need an equals and print, to satisfy the
  // GTSAM_CONCEPT_TESTABLE_INST(T) defined in Testable.h, but these are not needed below.

}; // UnaryFactor
*/
/* ************************************************************************* */


std::vector<cv::Point2f> keypoints1;
int i = 0;

//callback function
void mouseEvent1(int evt, int x, int y, int flags, void* param){
    cv::Mat *src1 = (cv::Mat*)param;
    cv::Point pot;
    //cv::imshow("src1",*src1);

    if (evt == CV_EVENT_LBUTTONDOWN && i<4){
        //keypoints1[i].pt.x = x;
        //keypoints1[i].pt.y = y;
        pot = cv::Point(x, y);
        cv::circle(*src1, pot, 5, cv::Scalar(0, 255, 0), 4, 5);

        keypoints1.push_back(cv::Point(x, y));
        printf("사각형의 %d번째 꼭지점의 좌표(%d, %d)\n", i + 1, x, y);
        cv::imshow("Image", *src1);
        i++;
    }
}

int main(int argc, char** argv)
{
    // Define the camera observation noise model
    noiseModel::Isotropic::shared_ptr measurementNoise = noiseModel::Isotropic::Sigma(3, 1.0);

    //vector<Pose3> poses = createPoses();

    ISAM2Params parameters;
    parameters.relinearizeThreshold = 0.01;
    parameters.relinearizeSkip = 1;
    ISAM2 isam(parameters);

    // Create a Factor Graph and Values to hold the new data
    NonlinearFactorGraph graph;
    Values initialEstimate;

    //input parameters of CLC : (fx, fy, cx, cy)
    CLC clc(300,300,512,384);
    std::vector<vector<Point2f> > squares;
    cv::Mat image;
    argv[1] ="/Users/jaemin/Dropbox/EigenEKFSLAM/EigenEKFSLAM/IMG_2658.JPG";
    argv[2] ="/Users/jaemin/Dropbox/EigenEKFSLAM/EigenEKFSLAM/IMG_2660.JPG";
    argv[3] ="/Users/jaemin/Dropbox/EigenEKFSLAM/EigenEKFSLAM/IMG_2664.JPG";
    argv[4] ="/Users/jaemin/Dropbox/EigenEKFSLAM/EigenEKFSLAM/IMG_2658.JPG";
    argv[5] =0;

    vector<Pose3> vecPoses, clcPoses;
    Pose3 currentPose;
    vecPoses.push_back(Pose3(Rot3::rodriguez(0, 0, 0), Point3(0, 0, 0)));
    for( int index = 1; argv[index] != 0; index++ )
    {
        cv::Mat original_image = cv::imread(argv[index], 1);
        if( original_image.empty() ){
            std::cout << "Couldn't load " << argv[index] << std::endl;
            return -1;
        }
        cv::resize(original_image, original_image, Size(1024, 768));



        initialEstimate.insert(Symbol('x', index-1), vecPoses[index-1].compose(Pose3(Rot3::rodriguez(0, 0, 0), Point3(0, 0, 0))));

        char inputFlag;
        do{
            keypoints1.clear();
            i = 0;
            original_image.copyTo(image);
            cv::namedWindow("Image", CV_WINDOW_AUTOSIZE);
            cv::imshow("Image", image);
                cv::setMouseCallback("Image", mouseEvent1, &image);

            inputFlag = cv::waitKey();
            if (keypoints1.empty()){
                std::cout << "error, no points are selected.\n";
                i=0;
                continue;
            }

            if (inputFlag == 'd'){
                continue;
            } else{
                //3. CLC Pose Calculation for each rectangle
                int ID_rect=0;
                cv::imshow("Image", original_image);
                std::cout << "Input ID of this rectangle : ";
                //std::cin >> ID_rect;
                ID_rect = 0;
                clc.SetOffCenteredQuad(keypoints1);
                clc.FindProxyQuadrilateral();
                Vector3d trans; Quaternion<double> q;
                if(!clc.CalcCLC(trans, q))
                {
                    std::cerr << "CLC is NaN" << std::endl;
                    continue;
                }

                Pose3 clcPose(Rot3::quaternion(q.w(),q.x(),q.y(),q.z()), Point3(trans[0], trans[1], trans[2])); // create a measurement for both factors (the same in this case)
                noiseModel::Diagonal::shared_ptr clcPoseNoise = noiseModel::Diagonal::Sigmas((gtsam::Vector(6) << gtsam::Vector3::Constant(0.3),gtsam::Vector3::Constant(0.1))); // 20cm std on x,y, 0.1 rad on theta
                graph.add(BetweenFactor<Pose3>(Symbol('x', index-1), Symbol('l', ID_rect), clcPose, clcPoseNoise));
                clcPoses.push_back(clcPose);
                clc.Visualization(image);
                cv::imshow("Image", image);
                cv::waitKey(1);
                inputFlag = cv::waitKey();
            }
        }while(inputFlag != 'f');
        inputFlag = 0;
        //4. EKF Correction
        if( index == 1) {
            // Add a prior on pose x0
            noiseModel::Diagonal::shared_ptr vecPoseNoise = noiseModel::Diagonal::Sigmas((gtsam::Vector(6) << Vector3::Constant(0.3),Vector3::Constant(0.1))); // 30cm std on x,y,z 0.1 rad on roll,pitch,yaw
            graph.push_back(PriorFactor<Pose3>(Symbol('x', 0), vecPoses[0], vecPoseNoise));

            // Add a prior on landmark l0
            noiseModel::Diagonal::shared_ptr clcPoseNoise = noiseModel::Diagonal::Sigmas((gtsam::Vector(6) << gtsam::Vector3::Constant(0.3),gtsam::Vector3::Constant(0.1))); // 20cm std on x,y, 0.1 rad on theta
            graph.push_back(PriorFactor<Pose3>(Symbol('l', 0), clcPoses[0], clcPoseNoise)); // add directly to graph

            // Add initial guesses to all observed landmarks
            // Intentionally initialize the variables off from the ground truth
            //for (size_t j = 0; j < clcPoses.size(); ++j)
            //initialEstimate.insert(Symbol('l', j), clcPoses[j].compose(Pose3(Rot3::rodriguez(0, 0, 0), Point3(0, 0, 0))));

        }else if( index == 2) {
            // Add a prior on pose x0
            noiseModel::Diagonal::shared_ptr vecPoseNoise = noiseModel::Diagonal::Sigmas((gtsam::Vector(6) << Vector3::Constant(0.3),Vector3::Constant(0.1))); // 30cm std on x,y,z 0.1 rad on roll,pitch,yaw
            graph.push_back(PriorFactor<Pose3>(Symbol('x', 1), vecPoses[0], vecPoseNoise));

            // Add a prior on landmark l0
            noiseModel::Diagonal::shared_ptr clcPoseNoise = noiseModel::Diagonal::Sigmas((gtsam::Vector(6) << gtsam::Vector3::Constant(0.3),gtsam::Vector3::Constant(0.1))); // 20cm std on x,y, 0.1 rad on theta
            graph.push_back(PriorFactor<Pose3>(Symbol('l', 1), clcPoses[1], clcPoseNoise)); // add directly to graph

            // Add initial guesses to all observed landmarks
            // Intentionally initialize the variables off from the ground truth
            for (size_t j = 0; j < clcPoses.size(); ++j)
            initialEstimate.insert(Symbol('l', j), clcPoses[j].compose(Pose3(Rot3::rodriguez(0, 0, 0), Point3(0, 0, 0))));

        } else {
          // Update iSAM with the new factors

            ISAM2Result result = isam.update(graph, initialEstimate);
            cout << "cliques: " << result.cliques << "\terr_before: " << *(result.errorBefore) << "\terr_after: " << *(result.errorAfter) << "\trelinearized: " << result.variablesRelinearized << endl;

          // Each call to iSAM2 update(*) performs one iteration of the iterative nonlinear solver.
          // If accuracy is desired at the expense of time, update(*) can be called additional times
          // to perform multiple optimizer iterations every step.
          isam.update();
          Values currentEstimate = isam.calculateEstimate();
          cout << "****************************************************" << endl;
          cout << "Frame " << i << ": " << endl;
          currentEstimate.print("Current estimate: ");
          vecPoses.push_back(Pose3(Rot3::rodriguez(0, 0, 0), Point3(0, 0, 0)));

          // Clear the factor graph and values for the next iteration
          graph.resize(0);
          initialEstimate.clear();
        }

    }

    return 0;
}
