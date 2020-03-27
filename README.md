# remote-sensor-mesh
DIY demo project as an implementation of Deployment and Operation of IoT devices in hazardous and remote areas

Based on publication by Allan Askar from February 28th, 2020

Problem: Operating cost of sensor network in remote and hazardous areas for monitoring is very high.

Solution: Deployment and Operation of IoT self-powered, lasting, low cost, small, wireless devices in hazardous and no connectivity environments to monitor level of hazard.

Example Use Case: Drone deployed mesh of devices for fire predicting and tracking. Drone fly by daily to get data from network. Drone deploys more devices at any time as desired for coverage/accuracy/additional data metrics.

Tenants:

    1. Power once:
        Device is powered once for the mission. 
        Powering initiates need for provisioning. 
        Provisioning performed with minimum touch in wireless mode.
    
    2. Sensors send data to nodes in async and session less mode. Datagram.

    3. Command and Control from Node to Sensor is Optional.

    4. No cross-node communication.

    5. No cross-sensor communication.

    6. Nodes act as Data Collector and network router.
        Node is responsible for sending data to the cloud when connectivity is available.

    7. While provisioning, support 'Reset the Mission' capability.

    8. Presence of the nodes in deployed environment could be intermittent.

    9. Fast provisioning and deployment of 1000s of devices.

Sensor loop:

    1. Init 'Blank' state if never executed. Ran once.
    2. Run 'Provisioning' if in 'Blank'. See details in R1.
    3. Run 'Reset Check' if in 'Testing' state. See R2.
    4. Set 'Mission' state if in 'Testing' state and RTC elapsed 1 hour. Lower power.
    5. Run 'Testing' code if in 'Testing' state.
    6. Run 'Mission' code if in 'Mission' state.

R1. Provisioning:

    1. Connect to available 'Admin' WiFi network.
    2. Open UDP server and get 'Config' sentence.
    3. Set configuration for a mission and 'Testing' state.
    4. Close server and connection.

R2. Reset Check:

    1. Connect to available 'Admin' WiFi network.
    2. Open UDP server and get 'Reset' sentence.
    3. Reset configuration and set 'Blank' state.
    4. Close server and connection.   

Testing and Mission sensor code loop:

    1. Read sensor(s).
    2. Store in RTC RAM into ring buffer.
    3. Try connect to any node in random.
    4. Send sensor data if connected.
    5. Initiate a Deep Sleep.
        Sleep period defers in 'Mission' vs 'Testing'.

Node is setup using direct automated or manual administration.

Sensor Admin device is setup using direct automated or manual administration.

FAQs:
    Q1. What is the possible use case?
        General use case is to solve the problem of monitoring remote areas with no access to power or connectivity on a low cost. Examples could be any industrial waste monitoring (usually it is remote and hazardous), or waterline monitoring in remote areas where water level may damage ecosystem.
    Q2. Why not to use long pooling connections?
        The sensor devices have limited battery life. Long pooling is a luxury in cases with limited gateway connectivity and limited battery.
    Q3. How long sensor data can stay at sensor with no node connectivity?
        It depends on usage. Ex: Temperature sensing once an hour and storing it in RTC 8Kb RAM as a single byte will allow 8K samples, which corresponds to 1h*8K=8000 hours = 333 days.
    Q4. How data gets to the cloud?
        It could be done several ways and all of them prioritize long zero-touch operation and low cost vs full connectivity. Ex1: Drone has a node onboard and fly into deployment zone every day to gather data from sensors or/and other nodes. Ex2: One and several nodes could have an LTE modem onboard and use it periodically. Ex3: Combination of Ex1 and Ex2.
    Q5. If node(s) is deployed, does mesh need to track which node has which sensor data?
        No. Nodes act as a means of delivering data only. Each sensor data message is unique. Data collected at the cloud could be easily processed to de-duplicate or further enhance.
    Q6. Does each sensor has to have a synchronized time clock?
        No. Sample rate and operation run time could be used to calculate time of sample. If node(s) deployed, they will have correct time clock and add that info to messages from sensors.
    Q7. Why not to use sensor to sensor data transfer to reach remote node?
        That will require coordination between sensors and more resources on radio operation on each which will consume more battery.
    Q8. Do sensors need to be of the same kind? Ex: Only sensing temperature or other metric?
        No. Each sensor has data sentence for many metrics to be send. Node operate on sentence level with need to know the schema of the sentence.
    Q9. What happens if sensor or node fails?
        This solution is designed for all possible failures. Dropping devices from sky may damage them. Sensor or node can stop functioning at any time and will stop functioning when battery depletes. All communication rules are structured around that principles.
    Q10.What is the typical cost for the sensor?
        Depends on a sensor type. In many general cases it will be less than $5.
    Q11.What is the typical cost for the node?
        In most cases it is around $15.
    Q12.Any estimated cost for network of deployed sensors/nodes to cover one squared mile of area?
        It depends on many parameters. In most cases we can assume 10-20 nodes and 200-400 sensors. This will cost $1150-$2300.
    Q13.Can devices be reused?
        Yes. After device is retrieved from the field and can easily be repurposed. In most of the cases it just need new battery and a new mission.
    Q14.What is 'Sensor Admin Device'?
        It could be any connected device with ability to run Wireless Access Point and host web server with sensor and mission configuration data. Ex: Mobile phone.
    Q15.What AWS services are used?
        Storage and analytical processing pipeline could use S3, Time-Series, Athena, Quicksight, Sagemaker. Sensors could be build on FreeRTOS. Nodes could run Greengrass. Ingest could use IoT Core, IoT Analytics.
    Q16.What is Sensor loop?
        Sensor loop is an algorithm representing the main process running on the sensor micro-controller in the infinite loop outlined in high-level steps.
    Q17.What is Provisioning?
        Provisioning is an algorithm representing a process running on sensor micro-controller inside ‘Provisioning’ sub routine.
    Q18.What is Mission?
        New single use of the devices mesh is a Mission. This way the same devices could be used in many missions. Mission one may have 2 Nodes and 100 Sensors deployed into field to monitor oil spill by the oil pipeline. Then, after this mission is over, the same devices could be collected and reused for next mission to monitor wild fire temperature in another field.
