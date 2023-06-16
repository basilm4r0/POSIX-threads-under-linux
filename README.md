# [POSIX-threads-under-linux](https://drive.google.com/file/d/1IiIrwXxUiTITYHnC0SMWzM9S8M4Aq6FB/view?usp=sharing)
Welcome to the Ant Searching for Food Simulation project! This project aims to simulate the behavior of a group of ants in search of food. The simulation is designed to be simple yet engaging, providing an opportunity to observe the collective behavior of ants.

## Objective
The objective of this project is to develop a multi-threaded simulation of ants searching for food. By leveraging multi-threading, our aim is to achieve a more efficient and realistic simulation that accurately represents the behavior of ants in their natural environment. Each ant is represented by a separate thread, enabling concurrent execution and independent movement of multiple ants. This implementation allows us to simulate complex behaviors, such as food detection, pheromone release, and coordinated movements toward food sources. With the use of threads, we can create an engaging and dynamic simulation that closely mimics the intricate behaviors observed in real ants during their search for food.

## Description  
- [Project Description](https://drive.google.com/file/d/1IiIrwXxUiTITYHnC0SMWzM9S8M4Aq6FB/view?usp=sharing)

 ## How 
 - Clone the repository to your local machine.
 - In treminal:  </br>
1- Make sure you have the libfreetype6-dev library installed   </br>
  ```
 sudo apt update
 sudo apt install libfreetype6-dev
 ```
&emsp; &ensp; 2- Then you can run the program using:

 ```
 cd POSIX-threads-under-linux/src
 make
 make run
 ```
 - By observing the simulation on the OpenGL screen, you can gain insights into the collective behavior of the ants, their coordination in locating food, and the propagation of social responses through pheromones.
 
## Configuration
You can customize the following parameters in the source code:

* Number of ants
* Speed range for ants
* Food placement interval
* Food portion size
* Number of food pieces
* Distance at which ants can smell food
* Weak pheromone threshold
* Strong pheromone threshold
* Ant turning angle when smelling pheromones
* Simulation duration

Feel free to modify these parameters in **conf.txt** to suit your specific needs.</br>
(*The maximum vlaue for __ variables is __, any greater values will be assumed __*)

## Languages And Tools:

- <img align="left" alt="Visual Studio Code" width="40px" src="https://raw.githubusercontent.com/github/explore/80688e429a7d4ef2fca1e82350fe8e3517d3494d/topics/visual-studio-code/visual-studio-code.png" /> <img align="left" alt=  "OpenGl" width="60px" src="https://upload.wikimedia.org/wikipedia/commons/e/e9/Opengl-logo.svg" /><img align="left" alt="C++" width="50px" src="https://upload.wikimedia.org/wikipedia/commons/1/18/ISO_C%2B%2B_Logo.svg" /><img align="left" alt="GitHub" width="50px" src="https://raw.githubusercontent.com/github/explore/78df643247d429f6cc873026c0622819ad797942/topics/github/github.png" /> <img align="left" alt="Linux" width="50px" src="https://upload.wikimedia.org/wikipedia/commons/thumb/3/35/Tux.svg/800px-Tux.svg.png" /> 

<br/>


## Team members :
- [Aseel Sabri](https://github.com/Aseel-Sabri)
- [Basil Mari] (https://github.com/basilm4r0)
- [Shahd Abu-Daghash](https://github.com/shahdDaghash)
- [Tala Alsweiti](https://github.com/talaalsweiti)
