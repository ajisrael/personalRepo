import processing.core.*; 
import processing.data.*; 
import processing.event.*; 
import processing.opengl.*; 

import peasy.*; 

import java.util.HashMap; 
import java.util.ArrayList; 
import java.io.File; 
import java.io.BufferedReader; 
import java.io.PrintWriter; 
import java.io.InputStream; 
import java.io.OutputStream; 
import java.io.IOException; 

public class personalRepo extends PApplet {



PeasyCam cam;

int dim = 3;
Cubie[] cube = new Cubie[dim*dim*dim];


Move move;

public void setup() {
  
  cam = new PeasyCam(this, 400);
  int index = 0;
  for (int x = -1; x <= 1; x++) {
    for (int y = -1; y <= 1; y++) {
      for (int z = -1; z <= 1; z++) {
        PMatrix3D matrix = new PMatrix3D();
        matrix.translate(x, y, z);
        cube[index] = new Cubie(matrix, x, y, z);
        index++;
      }
    }
  }

  move = new Move(0, 0, 1, 1);
}

public void turnZ(int index, int dir) {
  for (int i = 0; i < cube.length; i++) {
    Cubie qb = cube[i];
    if (qb.z == index) {
      PMatrix2D matrix = new PMatrix2D();
      matrix.rotate(dir*HALF_PI);
      matrix.translate(qb.x, qb.y);
      qb.update(round(matrix.m02), round(matrix.m12), qb.z);
      qb.turnFacesZ(dir);
    }
  }
}

public void turnY(int index, int dir) {
  for (int i = 0; i < cube.length; i++) {
    Cubie qb = cube[i];
    if (qb.y == index) {
      PMatrix2D matrix = new PMatrix2D();
      matrix.rotate(dir*HALF_PI);
      matrix.translate(qb.x, qb.z);
      qb.update(round(matrix.m02), qb.y, round(matrix.m12));
      qb.turnFacesY(dir);
    }
  }
}

public void turnX(int index, int dir) {
  for (int i = 0; i < cube.length; i++) {
    Cubie qb = cube[i];
    if (qb.x == index) {
      PMatrix2D matrix = new PMatrix2D();
      matrix.rotate(dir*HALF_PI);
      matrix.translate(qb.y, qb.z);
      qb.update(qb.x, round(matrix.m02), round(matrix.m12));
      qb.turnFacesX(dir);
    }
  }
}

public void keyPressed() {
  switch (key)
  {
  case 'f':
    turnZ(1, 1);
    break;
  case 'F':
    turnZ(1, -1);
    break;
  case 'b':
    turnZ(-1, 1);
    break;
  case 'B':
    turnZ(-1, -1);
    break;
  case 'u':
    turnY(-1, 1);
    break;
  case 'U':
    turnY(-1, -1);
    break;
  case 'd':
    turnY(1, 1);
    break;
  case 'D':
    turnY(1, -1);
    break;
  case 'l':
    turnX(-1, 1);
    break;
  case 'L':
    turnX(-1, -1);
    break;
  case 'r':
    turnX(1, 1);
    break;
  case 'R':
    turnX(1, -1);
    break;
  }
  move.start();
}

public void draw() {
  background(51);
  scale(50);

  move.update();

  for (int i = 0; i < cube.length; i++) {
    push();
    if (cube[i].z == move.z) {
      rotateZ(move.angle);
    }
    cube[i].show();
    pop();
  }
}
  public void settings() {  size(600, 600, P3D); }
  static public void main(String[] passedArgs) {
    String[] appletArgs = new String[] { "personalRepo" };
    if (passedArgs != null) {
      PApplet.main(concat(appletArgs, passedArgs));
    } else {
      PApplet.main(appletArgs);
    }
  }
}
