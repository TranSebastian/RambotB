#include "main.h"

//left side driver motors
pros::Motor left_mtr(1, true);
pros::Motor left_mtr2(2, true);

//right side drive motors
pros::Motor right_mtr(4);
pros::Motor right_mtr2(20);

//flywheel
pros::Motor flyWheelCW_mtr(5, pros::E_MOTOR_GEARSET_06);
pros::Motor flyWheelCCW_mtr(6, pros::E_MOTOR_GEARSET_06, true);

//intake
pros:: Motor frontIntake_mtr(7);

//feeder
pros:: Motor feeder_mtr(8);

//expansion
pros::ADIDigitalOut expand ('A');

//vision sensor
pros::Vision frontCam (9, pros::E_VISION_ZERO_CENTER);

//magazine
pros::ADIDigitalIn magazine ('B');




//startup code
void initialize() {

    expand.set_value(false);
    //tells the motors how to continue to spin until stopped
    flyWheelCW_mtr.set_brake_mode(pros::E_MOTOR_BRAKE_COAST);
    flyWheelCCW_mtr.set_brake_mode(pros::E_MOTOR_BRAKE_COAST);
}

void disabled() {}

void competition_initialize() {}

/*
    AUTONOMOUS METHODS
    - moveFor(int)
    - armsMoveFor(int, bool)
    - rotateTo(int, bool)
    - move(int, bool)
    - armsMove(int, bool)
*/

//moves robot for x ms forward or backwards
void moveFor (int ms, bool direction)
{
    int reverser = 1;
    if (!direction)
    {
        reverser = -1;
    }

    left_mtr.move_velocity(reverser*200);
    right_mtr.move_velocity(reverser*200);
    left_mtr2.move_velocity(reverser*200);
    right_mtr2.move_velocity(reverser*200);

    pros::delay(ms);
    left_mtr.move_velocity(0);
    right_mtr.move_velocity(0);
    left_mtr2.move_velocity(0);
    right_mtr2.move_velocity(0);
}

//moves the robot x inches forward or backwards
void move (double inches, bool direction)
{
    double time = ((inches/12.57) / (10/3))*1000;

    moveFor(time, direction);
}

//does caclulation to finds how long to rotate until positon is reached
void rotateTo (double degrees, bool direction)
{
    int reverser = 1;
    if (!direction){
        reverser = -1;
    }

    double distanceToRotate = degrees * (54.98/360);

    left_mtr.move_velocity(200*reverser);
    right_mtr.move_velocity(-200*reverser);

    left_mtr2.move_velocity(200*reverser);
    right_mtr2.move_velocity(-200*reverser);

    pros::delay(distanceToRotate*23.866348);
    left_mtr.move_velocity(0);
    right_mtr.move_velocity(0);
    left_mtr2.move_velocity(0);
    right_mtr2.move_velocity(0);
}

void toss (){

  //spin up the flywheel
  flyWheelCW_mtr.move_velocity(330);
  flyWheelCCW_mtr.move_velocity(-330);
  pros::delay(3000);

  //fire!!!!!!! WATCH OUT!!!
  feeder_mtr.move_velocity(300);
  pros::delay(2500);

  flyWheelCW_mtr.move_velocity(0);
  flyWheelCCW_mtr.move_velocity(0);
  feeder_mtr.move_velocity(0);

}

void moveToObject (){

  //stages of autonomous
  bool looking = true;

  //searching for goals
  while (looking){

    //colors
    pros::vision_signature_s_t wtf = pros::Vision::signature_from_utility(1, 7349,
      8835, 8092, -1595, -787, -1190, 3.000, 0);   //red
    pros::vision_signature_s_t alsowtf = pros::Vision::signature_from_utility(2,
      795, 971, 883, -4497, -4313, -4405, 4.100, 0); //yellow

    //find red object
    frontCam.set_signature(1, &wtf);
    pros::vision_object goal = frontCam.get_by_sig(0, 1);

    //goal is on left
    if (goal.x_middle_coord < -50){
        rotateTo(30, true);
    }
    //goal is on the right
    else if (goal.x_middle_coord >  50){
        rotateTo(30, false);
    }
    //shoot if its a high goal
    else if ( goal.y_middle_coord > -75) {
      toss();
      looking = false;
    }
    //rotate in case of nothing
    else {
      rotateTo(10, true);
    }

    pros::delay(20);
  }
}

//autonomous
void autonomous() {

  //roller
  move(5, true);
  frontIntake_mtr.move_velocity(200);
  pros::delay(2000);
  frontIntake_mtr.move_velocity(0);

  //prep for the vision sensor
  // move(12, false);
  // rotateTo(180, true);
  // moveToObject();

  moveFor(200, false);
  rotateTo(120, true);


  toss();




}

//operator controls
void opcontrol() {
    pros::Controller master(pros::E_CONTROLLER_MASTER);

    while (true) {

        //spin up the flywheel
        if (master.get_digital(pros::E_CONTROLLER_DIGITAL_L1) )
        {
            flyWheelCW_mtr.move_velocity(600);
            flyWheelCCW_mtr.move_velocity(-600);
        }
        else if (master.get_digital(pros::E_CONTROLLER_DIGITAL_L2) )
        {
            //"brakes"
            flyWheelCW_mtr.move_velocity(400);
            flyWheelCCW_mtr.move_velocity(-400);
        }
        else
        {
            flyWheelCW_mtr.move_velocity(0);
            flyWheelCCW_mtr.move_velocity(0);
        }

        //spin up intake
        if (master.get_digital(pros::E_CONTROLLER_DIGITAL_R1) )
        {
            frontIntake_mtr.move_velocity(-200);
        }
        else if (master.get_digital(pros::E_CONTROLLER_DIGITAL_R2) )
        {
            //"brakes"
            frontIntake_mtr.move_velocity(200);
        }
        else
        {
            frontIntake_mtr.move_velocity(0);
        }

        //feeder
        if (master.get_digital(pros::E_CONTROLLER_DIGITAL_B)){
            feeder_mtr.move_velocity(200);
        }
        else if (master.get_digital(pros::E_CONTROLLER_DIGITAL_X)){
            feeder_mtr.move_velocity(-200);
        }
        else {
          feeder_mtr.move_velocity(0);
        }

        //expansion!
        if (master.get_digital(pros::E_CONTROLLER_DIGITAL_UP)){
          expand.set_value(true);

        }
        else if (master.get_digital(pros::E_CONTROLLER_DIGITAL_DOWN)){
          expand.set_value(false);

        }

        /*
          arcade control
          works by taking y axis value and either adding or subtracting x axis
          value, then dividing by 2

          Ex: (82.731, 82.731): position when stick is moved to 45 degrees
            move leftMtr 124.1
            move rightMtr 41.4
            causes the robot to move in bank to right

          Different coordinates in different quandrants moves into that direction
            II  (-82.731, 82.731)   --> bank into left
            III (-82.731, -82.731)  --> backwards bank into right
            IV  (82.731, -82.731)   --> backwards bank into left

        */

        left_mtr.move(master.get_analog(ANALOG_LEFT_Y) - master.get_analog(ANALOG_LEFT_X)/2);
        right_mtr.move (master.get_analog(ANALOG_LEFT_Y) + master.get_analog(ANALOG_LEFT_X)/2);



        left_mtr2.move(master.get_analog(ANALOG_LEFT_Y) - master.get_analog(ANALOG_LEFT_X)/2);
        right_mtr2.move (master.get_analog(ANALOG_LEFT_Y) + master.get_analog(ANALOG_LEFT_X)/2);




        pros::delay(20);
    }
}
