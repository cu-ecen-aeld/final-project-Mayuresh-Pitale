# Smart Edge Sensor Node with Local Machine Learning Inference 

**Student:** Mayuresh Rajesh Pitale  
**Course:** Advanced Embedded Software Development 

Welcome to my final project repository! This repository serves as the individual submission tracker and the primary host for the **custom C/C++ user-space application** and the **kernel driver source code**.

---

## 🎯 Project Goals and Motivation
In modern industrial settings, transmitting continuous, high-frequency machine vibration data to a central cloud server is bandwidth-intensive and introduces significant latency. 

The goal of this project is to build a localized predictive maintenance edge node. It captures high-frequency vibration data via a custom character driver, passes it through a secure Inter-Process Communication (IPC) pipeline, and runs a lightweight Machine Learning Autoencoder locally to detect anomalies. By processing data directly on the edge, the system drastically reduces network overhead and ensures immediate fault detection.

---

## 🔗 Quick Navigation Links

All architectural details, hardware specifications, and sprint tracking can be found at the links below:

* 📖 **[Complete Project Overview Wiki Page](https://github.com/cu-ecen-aeld/final-project-Mayuresh-Pitale/wiki/Project-Overview)**
  * *The single source of truth for system architecture, hardware schematics, and project details.*

* 📅 **[Project Schedule & Sprint Kanban Board](https://github.com/users/Mayuresh-Pitale/projects/4)**
  * *Live tracking of all Sprints, Issues, and Definition of Done (DoD) deliverables.*

### Source Code Repositories

To adhere to best practices for Embedded Linux development, the source code and OS build environments are strictly separated:

* 💻 **[Application & Driver Source Code](https://github.com/cu-ecen-aeld/final-project-Mayuresh-Pitale)**
  * *This current repository hosts the pure C Linux kernel character driver (`MPU6050_driver.c`) and the multithreaded user-space application.*

* ⚙️ **[OS Build System (Buildroot) Repository](https://github.com/Mayuresh-Pitale/edge-node-buildroot)**
  * *Contains the custom `defconfig`, device tree overlays, and package makefiles used to cross-compile the custom Linux OS image.*
