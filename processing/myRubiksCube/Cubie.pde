class Cubie {
  PMatrix3D matrix;
  int x = 0;
  int y = 0;
  int z = 0;
  Face[] faces;

  Cubie(PMatrix3D m, int x, int y, int z) {
    this.matrix = m;
    this.x = x;
    this.y = y;
    this.z = z;

    faces = getFaces();
  }

  Face[] getFaces() {
    Face[] currentFaces = new Face[6];
    if (this.x == -1*limit) { 
      currentFaces[0] = new Face(new PVector(-1, 0, 0), color(255, 0, 0));
      currentFaces[1] = new Face(new PVector( 1, 0, 0), blackout);
    } else {
      currentFaces[0] = new Face(new PVector(-1, 0, 0), blackout);
      currentFaces[1] = new Face(new PVector(1, 0, 0), color(255, 128, 0));
    }

    if (this.y == -1*limit) { 
      currentFaces[2] = new Face(new PVector(0, -1, 0), color(255, 255, 0));
      currentFaces[3] = new Face(new PVector(0, 1, 0), blackout);
    } else {
      currentFaces[2] = new Face(new PVector(0, -1, 0), blackout);
      currentFaces[3] = new Face(new PVector(0, 1, 0), color(255));
    }

    if (this.z == -1*limit) {
      currentFaces[4] = new Face(new PVector(0, 0, -1), color(0, 0, 255));
      currentFaces[5] = new Face(new PVector(0, 0, 1), blackout);
    } else {
      currentFaces[4] = new Face(new PVector(0, 0, -1), blackout);
      currentFaces[5] = new Face(new PVector(0, 0, 1), color(0, 255, 0));
    }
    return currentFaces;
  }

  void turnFacesZ(int dir) {
    for (Face f : faces) {
      f.turnZ(dir * HALF_PI);
    }
  }

  void turnFacesY(int dir) {
    for (Face f : faces) {
      f.turnY(dir * HALF_PI);
    }
  }

  void turnFacesX(int dir) {
    for (Face f : faces) {
      f.turnX(dir * HALF_PI);
    }
  }

  void update(int x, int y, int z) {
    matrix.reset();
    matrix.translate(x, y, z);
    this.x = x;
    this.y = y;
    this.z = z;
  }

  void show() {
    noFill();
    stroke(0);
    strokeWeight(0.1);
    pushMatrix();
    applyMatrix(matrix);
    box(1);
    for (Face f : faces) {
      f.show();
    }
    popMatrix();
  }
}
