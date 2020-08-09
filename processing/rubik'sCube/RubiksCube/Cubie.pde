class Cubie {
  PMatrix3D matrix;
  boolean highlight = false;

  Cubie(PMatrix3D m) {
    matrix = m;
  }

  void show() {
    fill(255);
    
    stroke(0);
    strokeWeight(0.1);
    pushMatrix();
    applyMatrix(matrix);
    box(1);
    popMatrix();
  }
}
