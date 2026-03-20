# Minecraft 3D Maze with SOR Modeler

A C++ graphics project that combines a custom SOR modeler with a first-person 3D maze game built using OpenGL and GLUT.

## Overview
This project was developed as a computer graphics project in C++.  
It consists of two major parts:

- a custom SOR (Surface of Revolution) modeler for creating 3D objects
- a first-person 3D maze game that uses generated models in an interactive environment

The project covers the full workflow from 3D model creation to real-time gameplay.

## Main Features
- First-person 3D maze exploration
- Real-time player movement and camera control
- Collision detection
- Texture mapping and lighting
- Sound effects and on-screen text
- Custom SOR modeling tool
- Vertex editing and rotational surface generation
- Wireframe and polygon mesh rendering
- Model export for external execution

## Tech Stack
- C++
- OpenGL
- GLUT / FreeGLUT
- Computer Graphics

## Project Structure
This project includes:
- 3D maze game logic
- rendering and camera control
- collision handling
- texture and lighting systems
- SOR model generation
- mesh construction and editing tools

## How to Run
- Clone this repository or download the source code
- Open the project in Visual Studio
- Make sure OpenGL and GLUT / FreeGLUT are installed and configured correctly
- Build the project
- Run the executable

## Requirements
Before running the project, make sure the following are installed:

- C++ compiler
- OpenGL
- GLUT / FreeGLUT
- A development environment such as Visual Studio

## FreeGLUT Setup Notes
If FreeGLUT is not already configured on your system, you may need to:

- install FreeGLUT
- add the FreeGLUT header files to your compiler include path
- link the required library files
- place the necessary DLL file in the executable directory or system path

Typical files include:

- `GL/freeglut.h`
- `freeglut.lib`
- `freeglut.dll`

## Learning Outcomes
Through this project, I practiced:
- building interactive 3D graphics applications in C++
- implementing real-time rendering and movement systems
- designing custom 3D modeling workflows
- applying OpenGL concepts such as transformations, lighting, and texture mapping
- integrating graphics tool development with gameplay systems

## Author
Eric Hyunwoo Sung
