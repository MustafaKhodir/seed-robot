#!/usr/bin/env python3
# Autonomous driving for the seed robot (TurtleBot3, ROS2).
#
# The robot drives forward slowly and uses the laser distance sensor (LDS) to
# avoid anything that gets within 0.2 m. If something is straight ahead or just
# off to either side it stops and turns on the spot until the way is clear, then
# carries on. While it drives the Arduino on top keeps dropping seeds.
#
# Run it after the TurtleBot bringup is up:
#   python3 autonomous_lds.py
# Ctrl+C stops the robot and saves the last odometry reading to a text file.

import math
import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
from nav_msgs.msg import Odometry
from sensor_msgs.msg import LaserScan


class PIDController:
    # Standard PID for smoothing the forward speed. See the note in
    # timer_callback for how it connects to the drive.
    def __init__(self, Kp, Ki, Kd, setpoint):
        self.Kp = Kp  # Proportional gain
        self.Ki = Ki  # Integral gain
        self.Kd = Kd  # Derivative gain
        self.setpoint = setpoint
        self.prev_error = 0  # previous error, needed for the derivative term
        self.integral = 0    # running total of the error for the integral term

    def compute(self, current_value):
        error = self.setpoint - current_value
        self.integral += error
        derivative = error - self.prev_error
        self.prev_error = error  # keep it for the next call
        output = (self.Kp * error) + (self.Ki * self.integral) + (self.Kd * derivative)
        return output


class MyRobot(Node):
    def __init__(self):
        super().__init__('subscribe_laser_node')

        # The LDS publishes with best effort reliability, so the subscription
        # has to ask for the same or it never receives anything
        qos_policy = rclpy.qos.QoSProfile(
            reliability=rclpy.qos.ReliabilityPolicy.BEST_EFFORT,
            history=rclpy.qos.HistoryPolicy.KEEP_LAST,
            depth=1)

        self.sub = self.create_subscription(LaserScan, 'scan', self.scan_callback, qos_profile=qos_policy)
        self.sub2 = self.create_subscription(Odometry, 'odom', self.odom_callback, 10)
        self.pub = self.create_publisher(Twist, 'cmd_vel', 10)

        timer_period = 0.3  # seconds between control decisions
        self.timer = self.create_timer(timer_period, self.timer_callback)

        self.my_x = 0    # x position from odometry
        self.my_y = 0    # y position from odometry
        self.my_th = 0   # heading angle in degrees
        self.my_ls = LaserScan()      # latest laser scan
        self.obstacle_range = 0.2     # metres, anything closer than this is an obstacle
        self.forward_speed = 0.05     # m/s

        # PID controller parameters
        self.Kp = 1.0
        self.Ki = 0.1
        self.Kd = 0.01
        self.setpoint = 0.0
        self.pid_controller = PIDController(self.Kp, self.Ki, self.Kd, self.setpoint)

    # Keeps the newest set of laser ranges
    def scan_callback(self, msg):
        self.my_ls.ranges = msg.ranges

    # Keeps the newest position and converts the quaternion to a heading in degrees
    def odom_callback(self, msg):
        self.my_x = msg.pose.pose.position.x
        self.my_y = msg.pose.pose.position.y
        q0 = msg.pose.pose.orientation.w
        q1 = msg.pose.pose.orientation.x
        q2 = msg.pose.pose.orientation.y
        q3 = msg.pose.pose.orientation.z
        self.my_th = math.atan2(2 * (q0 * q3 + q1 * q2), (1 - 2 * (q2 * q2 + q3 * q3))) * 180 / math.pi

    # Runs every 0.3 s and decides what the robot does next
    def timer_callback(self):
        move = Twist()

        # Nothing to decide until the first scan has arrived
        if len(self.my_ls.ranges) == 0:
            return

        front = self.my_ls.ranges[0]
        side_a = self.my_ls.ranges[30]
        side_b = self.my_ls.ranges[270]

        if front < self.obstacle_range:
            # obstacle straight ahead, stop and turn
            move.angular.z = 0.5
            move.linear.x = 0.0
        elif side_a < self.obstacle_range:
            # obstacle just off to one side, turn a bit harder
            move.angular.z = 0.7
            move.linear.x = 0.0
        elif side_b < self.obstacle_range:
            # obstacle just off to the other side
            move.angular.z = 0.7
            move.linear.x = 0.0
        else:
            # path is clear, drive forward.
            # The PID output is calculated every cycle and is ready to drive
            # linear.x. During testing I kept the speed fixed at 0.05 m/s.
            pid_output = self.pid_controller.compute(self.my_x)
            move.linear.x = self.forward_speed
            move.angular.z = 0.0

        self.pub.publish(move)

        print('x = {0:5.2f}, y = {1:5.2f}, th = {2:5.2f}'.format(self.my_x, self.my_y, self.my_th))
        print('front = {0:5.2f}, side a = {1:5.2f}, side b = {2:5.2f}'.format(front, side_a, side_b))

    # Saves the last odometry reading so I could compare runs afterwards
    def save_readings(self):
        with open('laserscan_trajectory.txt', 'w') as myfile:
            myfile.write('Last odometry reading before shutdown\n')
            myfile.write(f'Linear X: {self.my_x}\n')
            myfile.write(f'Linear Y: {self.my_y}\n')
            myfile.write(f'Heading Angle: {self.my_th}\n')
        print('Readings saved to laserscan_trajectory.txt')

    def stop_robot(self):
        move = Twist()
        move.linear.x = 0.0
        move.angular.z = 0.0
        self.pub.publish(move)
        print('Shutdown')


def main(args=None):
    rclpy.init(args=args)
    myrobot_node = MyRobot()
    try:
        rclpy.spin(myrobot_node)
    except KeyboardInterrupt:
        pass
    finally:
        myrobot_node.stop_robot()
        myrobot_node.save_readings()
        myrobot_node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
