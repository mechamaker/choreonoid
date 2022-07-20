#include <cnoid/SimpleController>
#include <cnoid/Link>
#include <cnoid/AccelerationSensor>
#include <cnoid/RateGyroSensor>
#include <vector>

using namespace cnoid;
//南雲
const double F[] = {0, -1.7711, -0.0317, -0.3206}; //Φ θ dΦ dθ      　θ dθ dΦ

//計測結果に基づくやつ
//const double F[] = {0, -1.7711, -0.0317, -0.3206}; //Φ θ dΦ dθ      　θ dθ dΦ

//仕様書に基づくやつ
//const double F[] = {0, -1.7711, -0.0317, -0.3206}; //Φ θ dΦ dθ      　θ dθ dΦ

class PendulumController : public cnoid::SimpleController
{
    SimpleControllerIO* io;
    RateGyroSensor* GyroSensor;
    Link* body;
    Link* wheel_right;
    Link* wheel_left;

    double theta;
    double phi;
    double d_theta;
    double d_phi;
    double motor_vel;

public:

    virtual bool initialize(SimpleControllerIO* io) override
    {
        GyroSensor = io->body()->findDevice<RateGyroSensor>("GyroSensor");
        io->enableInput(GyroSensor);

        body = io->body()->link("body");
        io->enableInput(body);
        body->setActuationMode(Link::StateNone);

        wheel_right = io->body()->link("wheel_right");
        io->enableInput(wheel_right, JointAngle | JointVelocity);
        wheel_right->setActuationMode(Link::JointVelocity);
        io->enableOutput(wheel_right);

        wheel_left = io->body()->link("wheel_left");
        io->enableInput(wheel_left, JointAngle | JointVelocity);
        wheel_left->setActuationMode(Link::JointVelocity);
        io->enableOutput(wheel_left);

        return true;
    }

    virtual bool control() override
    {
        Vector3 w = GyroSensor->w();

        theta += w.x();
        d_theta = w.x();

        phi = wheel_right->q();
        d_phi = wheel_right->dq();

        motor_vel = F[0]*phi + F[1]*theta + F[2]*d_phi + F[3]*d_theta;

        wheel_right->dq_target() = motor_vel;
        wheel_left->dq_target() = motor_vel;

        return true;
    }
};

CNOID_IMPLEMENT_SIMPLE_CONTROLLER_FACTORY(PendulumController)