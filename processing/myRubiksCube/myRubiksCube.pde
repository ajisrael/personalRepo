import peasy.*;

PeasyCam cam;

float speed = 0.2;

color blackout = color(0);
int dim = 3;
int limit = dim / 2;
boolean even = (0 == dim % 2);
boolean autosolve = false;

Cubie[] cube = new Cubie[dim*dim*dim];

Move[] allMoves = new Move[] {
  new Move(0, 1, 0, 1), // D'
  new Move(0, 1, 0, -1), // D
  new Move(0, -1, 0, 1), // U
  new Move(0, -1, 0, -1), // U'
  new Move(1, 0, 0, 1), // R
  new Move(1, 0, 0, -1), // R'
  new Move(-1, 0, 0, 1), // L'
  new Move(-1, 0, 0, -1), // L
  new Move(0, 0, 1, 1), // F
  new Move(0, 0, 1, -1), // F'
  new Move(0, 0, -1, 1), // B'
  new Move(0, 0, -1, -1), // B
};

ArrayList<Move> sequence = new ArrayList<Move>();

Move currentMove;
int counter = 0;


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

  for (int i = 0; i < 50; i++) {
    int r = int(random(allMoves.length));
    Move m = allMoves[r];
    sequence.add(m);
  }

  currentMove = sequence.get(counter);

  if (autosolve) {
    for (int i = sequence.size()-1; i >= 0; i--) {
      Move nextMove = sequence.get(i).copy();
      nextMove.reverse();
      sequence.add(nextMove);
    }
  }
}

void draw() {
  background(51);

  // Setup position of cube at an angle
  rotateX(-0.5);
  rotateY(0.4);
  rotateZ(0.1);

  scale(50);

  currentMove.update();
  if (currentMove.finished()) {
    if (counter < sequence.size()-1) {
      counter++;
      currentMove = sequence.get(counter);
      currentMove.start();
    }
  }

  for (int i = 0; i < cube.length; i++) {
    push();
    if (abs(cube[i].z) > 0 && cube[i].z == currentMove.z) {
      rotateZ(currentMove.angle);
    } else if (abs(cube[i].x) > 0 && cube[i].x == currentMove.x) {
      rotateX(currentMove.angle);
    } else if (abs(cube[i].y) > 0 && cube[i].y == currentMove.y) {
      rotateY(-currentMove.angle);
    }
    cube[i].show();
    pop();
  }
}
