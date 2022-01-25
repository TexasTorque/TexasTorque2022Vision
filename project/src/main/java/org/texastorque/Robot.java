package org.texastorque;

import org.texastorque.torquelib.base.*;

import java.util.ArrayList;

import javax.lang.model.type.DeclaredType;

import org.texastorque.inputs.*;
import org.texastorque.inputs.State.RobotState;
import org.texastorque.subsystems.*;

import edu.wpi.first.networktables.NetworkTable;
import edu.wpi.first.networktables.NetworkTableEntry;
import edu.wpi.first.networktables.NetworkTableInstance;
import edu.wpi.first.wpilibj.smartdashboard.SmartDashboard;

public class Robot extends TorqueIterative {

    Input input = Input.getInstance();
    Feedback feedback = Feedback.getInstance();
    State state = State.getInstance();

    ArrayList<TorqueSubsystem> subsystems = new ArrayList<TorqueSubsystem>();

    NetworkTable ballDetectionNetworkTable = NetworkTableInstance.getDefault().getTable("ball_detection");

    @Override
    public void robotInit() {
    }

    @Override
    public void alwaysContinuous() {
        //feedback.update();
        //feedback.smartDashboard();
        subsystems.forEach(TorqueSubsystem::updateSmartDashboard);

        ballDetectionNetworkTable.getEntry("alliance_color").setString("blue");
        double pos = ballDetectionNetworkTable.getEntry("ball_horizontal_position").getDouble(0);
        double fps = ballDetectionNetworkTable.getEntry("frames_per_second").getDouble(-1);

        SmartDashboard.putNumber("POS", pos);
        SmartDashboard.putNumber("FPS", fps);

        //System.out.println(PullFromNT());
    }
    
    @Override
    public void disabledInit() {
        state.setRobotState(RobotState.DISABLED);
        subsystems.forEach(TorqueSubsystem::disable);
    }

    @Override
    public void teleopInit() {
        state.setRobotState(RobotState.TELEOP);
        subsystems.forEach(TorqueSubsystem::initTeleop);
    }

    @Override
    public void teleopContinuous() {
        input.update();
        input.smartDashboard();
        subsystems.forEach(TorqueSubsystem::updateTeleop);
        subsystems.forEach(TorqueSubsystem::output);
    }

    @Override
    public void autoInit() {
        state.setRobotState(RobotState.AUTONOMOUS);
        subsystems.forEach(TorqueSubsystem::initAuto);
    }

    @Override
    public void autoContinuous() {
        subsystems.forEach(TorqueSubsystem::updateAuto);
        subsystems.forEach(TorqueSubsystem::output);
    }

    @Override
    public void endCompetition() {
        System.out.printf("     _______              _______                 \n"
        + "    |__   __|            |__   __|                                \n"
        + "       | | _____  ____ _ ___| | ___  _ __ __ _ _   _  ___         \n"
        + "       | |/ _ \\ \\/ / _` / __| |/ _ \\| '__/ _` | | | |/ _ \\    \n"
        + "       | |  __/>  < (_| \\__ \\ | (_) | | | (_| | |_| |  __/      \n"
        + "       |_|\\___/_/\\_\\__,_|___/_|\\___/|_|  \\__, |\\__,_|\\___| \n"
        + "                                            | |                   \n"
        + "                                            |_|                   \n");
    }
}
