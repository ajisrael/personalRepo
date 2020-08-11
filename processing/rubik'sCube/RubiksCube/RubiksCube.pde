import peasy.*;

PeasyCam cam;

int dim = 3;
Cubie[] cube = new Cubie[dim*dim*dim];

void setup() {
  size(600, 600, P3D);
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
}

void turnZ(int index, int dir) {
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

void turnY(int index, int dir) {
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

void turnX(int index, int dir) {
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

void keyPressed() {
  switch (key)
  {
  case 'f':
    turnZ(-1, 1);
    break;
  case 'F':
    turnZ(-1, -1);
  case 'b':
    turnZ(1, 1);
    break;
  case 'B':
    turnZ(1, -1);
    break;
  case 'u':
    turnY(-1, 1);
    break;
  case 'U':
    turnY(-1, -1);
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
}

void draw() {
  background(51);
  scale(50);
  for (int i = 0; i < cube.length; i++) {
    cube[i].show();
  }
}
