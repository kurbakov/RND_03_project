#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // TODO: Request a service and pass the velocities to it to drive the robot
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;
    
    // Call the command_robot service and pass the requested move commands
    if(!client.call(srv)){
        ROS_ERROR("Failed to call service command_robot");
    }
}

// This callback function continuously executes and reads the image data
// ROS message: https://docs.ros.org/melodic/api/sensor_msgs/html/msg/Image.html
void process_image_callback(const sensor_msgs::Image img)
{

    const int white_pixel = 255;
    int leftSideCount = 0;
    int midSideCount = 0;
    int rightSideCount = 0;

    const int leftRange = img.width/3;
    const int rightRange = leftRange*2;

    // from image ros message weget that data has lenght width * step
    // += 3 because we want to find RGB (255,255,255)
    for(size_t i=0; i< img.height*img.step; i += 3){
        // TODO: Loop through each pixel in the image and check if there's a bright white one
        if(img.data[i] == white_pixel && img.data[i+1] == white_pixel && img.data[i+2] == white_pixel){
            // Then, identify if this pixel falls in the left, mid, or right side of the image
            const int position =  i % (img.width * 3) / 3;
            if(position > rightRange){
                ++rightSideCount;
            }
            else if(position < leftRange){
                ++leftSideCount;
            }
            else{
                ++midSideCount;
            }
        }
    }

    const int max = std::max({leftSideCount, midSideCount, rightSideCount});

    if(max > 0){
        // Depending on the white ball position, call the drive_bot function and pass velocities to it
        if(max == leftSideCount){
            // rotate left
            drive_robot(0.0, 1.0);
        }
        else if(max == rightSideCount){
            // rotate right
            drive_robot(0.0, -1.0);
        }
        else{
            // go straight
            drive_robot(1.0, 0.0);
        }
    }
    else{
        // Request a stop when there's no white ball seen by the camera
        drive_robot(0,0); 
    }

}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}