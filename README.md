# -CW2-Extended-Autonomous-Reliable-Car
Coursework Assignment CW2: Extended Autonomous Reliable Car

This coursework is meant to introduce students to design techniques being important for embedded systems
design, ranging from resource awareness to safety analysis. At the same time, it is also about to further
practice with the programming of sensors and actuators of an embedded platform. The programming language
to be used is ISO C. The embedded platform is the robot car Initio from 4Tronix.
The proposed application is to write software for the initio car so that it acts as an autonomous car following
another car in front of it. For simplicity, the car to be followed is simulated by a manually held sheet of paper
with a region of a specific colour on a white background. The car software should make sure that it follows the
simulated car with a certain safety distance and not come too close. To realise the full potential of that
application, the car should also use its on-board camera besides the other sensors, to obtain better awareness
of the environment. If anything unexpected happens (sudden close object, etc.) the car should switch into a
safe case mode by just stopping
