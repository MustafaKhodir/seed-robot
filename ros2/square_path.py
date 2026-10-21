#!/usr/bin/env python3
# Square path test for the seed robot (TurtleBot3, ROS2).
#
# This was my first driving test before the laser avoidance was written. The
# robot drives a 1 m square using odometry only: forward until x passes 1 m,
# turn left until the heading passes 90 degrees, forward until y passes 1 m,
# and so on round the square, then stops. The variable q is the step number.
#
# Run it after the TurtleBot bringup is up:
#   python3 square_path.py

import math
import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
from nav_msgs.msg import Odometry


class SquarePath(Node):
    def __init__(self):
        super().__init__('square_path_node')
        self.sub = self.create_subscription(Odometry, 'odom', self.odom_callback, 10)
        self.pub = self.create_publisher(Twist, 'cmd_vel', 10)
        self.timer = self.create_timer(0.3, self.timer_callback)

        self.q = 0       # which leg of the square we are on
        self.my_x = 0
        self.my_y = 0
        self.my_th = 0   # heading in degrees, -180 to 180

    def odom_callback(self, msg):
        self.my_x = msg.pose.pose.position.x
        self.my_y = msg.pose.pose.position.y
        q0 = msg.pose.pose.orientation.w
        q1 = msg.pose.pose.orientation.x
        q2 = msg.pose.pose.orientation.y
        q3 = msg.pose.pose.orientation.z
        self.my_th = math.atan2(2 * (q0 * q3 + q1 * q2), (1 - 2 * (q2 * q2 + q3 * q3))) * 180 / math.pi

    def timer_callback(self):
        move = Twist()

        if self.q == 0:                 # leg 1, drive forward along x
            move.linear.x = 0.20
            if self.my_x > 1.00:
                self.q = 1
        elif self.q == 1:               # corner 1, turn until facing 90 degrees
            move.angular.z = 0.2
            if self.my_th > 90.00:
                self.q = 2
        elif self.q == 2:               # leg 2, drive forward along y
            move.linear.x = 0.20
            if self.my_y > 1.00:
                self.q = 3
        elif self.q == 3:               # corner 2, turn until facing 180 degrees
            move.angular.z = 0.2
            if self.my_th > 177 or self.my_th < -170:
                self.q = 4
        elif self.q == 4:               # leg 3, drive back along x
            move.linear.x = 0.20
            if self.my_x < 0.00:
                self.q = 5
        elif self.q == 5:               # corner 3, turn until facing -90 degrees
            move.angular.z = 0.2
            if self.my_th > -90.00:
                self.q = 6
        elif self.q == 6:               # leg 4, drive back along y to the start
            move.linear.x = 0.20
            if self.my_y < 0.05:
                self.q = 7
        else:                           # done, stay stopped
            move.linear.x = 0.0
            move.angular.z = 0.0

        self.pub.publish(move)
        print('x = {0:5.2f}, y = {1:5.2f}, th = {2:5.2f}, q = {3:d}'.format(self.my_x, self.my_y, self.my_th, self.q))

    def stop_robot(self):
        self.pub.publish(Twist())
        print('Shutdown')


def main(args=None):
    rclpy.init(args=args)
    node = SquarePath()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.stop_robot()
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
