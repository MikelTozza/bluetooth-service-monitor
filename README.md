# bluetooth-service-monitor
=======
The bluetooth-service-monitore is a background service designed to enhance audio experiences on Windows devices by managing Bluetooth handsfree services.
This project focuses on monitoring audio quality in real-time, specifically detecting when audio becomes muffled or degraded during application usage.
Upon detection, it automatically adjusts Bluetooth handsfree settings to ensure optimal audio clarity and performance.

Key Features:
- Real-time Audio Monitoring: Continuously analyzes audio output to detect quality issues such as muffled sound, particularly when different applications are running.
- Service Management: Automatically toggles the Bluetooth handsfree service on or off based on the audio quality detected, aiming to mitigate issues without user intervention.
- Background Operation: Runs silently in the background, requiring minimal resources, and operates transparently to the user, enhancing usability without disrupting ongoing tasks.

This project is ideal for users who frequently encounter audio issues with Bluetooth-enabled devices on Windows,
offering a seamless solution to improve audio interaction quality in various software environments
==============================
COMING SOON:
- Right now the audio detection is being transcoded from python to c++ to reduce the size in the dev environment 
