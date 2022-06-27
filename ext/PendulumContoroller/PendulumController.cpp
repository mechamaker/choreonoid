#include <cnoid/SimpleController>
#include <cnoid/Link>
#include <vector>

using namespace cnoid;

const double F[] = {1.25, 28.637, 1.6504, 4.2012};

class PendulumController : public cnoid::SimpleController
{
    Link* body;
    Link* wheel_right;
    Link* wheel_left;

    double xc;
    double dxc;
    double qp;
    double dqp;
    double torque;

public:

    virtual bool initialize(SimpleControllerIO* io) override
    {
        body = io->body()->link("body");
        io->enableInput(body, JOINT_DISPLACEMENT | JOINT_VELOCITY);
        body->setActuationMode(Link::StateNone);


        wheel_right = io->body()->link("wheel_right");
        io->enableInput(wheel_right, JOINT_ANGLE | JOINT_VELOCITY);
        wheel_right->setActuationMode(Link::JointTorque);
        io->enableOutput(wheel_right);

        wheel_left = io->body()->link("wheel_left");
        io->enableInput(wheel_left, JOINT_ANGLE);
        wheel_left->setActuationMode(Link::JointTorque);
        io->enableOutput(wheel_left);

        return true;
    }

    virtual bool control() override
    {
        xc = wheel_right->q() * 0.024;
        dxc = wheel_right->dq() * 0.024;
        qp = body->q();
        dqp = body->dq();

        torque = F[0]*xc + F[1]*qp + F[2]*dxc + F[3]*dqp;

        wheel_right->u() = torque;

        return true;

    }
};

CNOID_IMPLEMENT_SIMPLE_CONTROLLER_FACTORY(PendulumController)