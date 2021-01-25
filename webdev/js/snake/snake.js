function Snake() {
  this.x;
  this.y;
  this.xSpeed = 0;
  this.ySpeed = 0;
  this.total = 0;
  this.tail = [];

  this.pickLocation = function () {
    this.x = (Math.floor(Math.random() * rows - 1) + 1) * scale;
    this.y = (Math.floor(Math.random() * columns - 1) + 1) * scale;
  };

  this.draw = function () {
    ctx.fillStyle = "#3b7a2a";

    for (let i = 0; i < this.tail.length; i++) {
      ctx.fillRect(this.tail[i].x, this.tail[i].y, scale, scale);
    }

    ctx.fillRect(this.x, this.y, scale, scale);
  };

  this.update = function () {
    for (let i = 0; i < this.tail.length - 1; i++) {
      this.tail[i] = this.tail[i + 1];
    }

    this.tail[this.total - 1] = { x: this.x, y: this.y };

    this.x += this.xSpeed;
    this.y += this.ySpeed;

    // if (this.x > (canvas.width-scale) || this.x < 0 || this.y > (canvas.height-scale) || this.y < 0)
    // {
    //   this.reset();
    // }

    if (this.x > canvas.width - scale) {
      this.x = 0;
    }
    if (this.x < 0) {
      this.x = canvas.width - scale;
    }
    if (this.y > canvas.height - scale) {
      this.y = 0;
    }
    if (this.y < 0) {
      this.x = canvas.height - scale;
    }

    for (let i = 0; i < this.tail.length; i++) {
      if (this.x === this.tail[i].x && this.y === this.tail[i].y) {
        this.reset();
      }
    }
  };

  this.changeDirection = function (direction) {
    switch (direction) {
      case "Up":
        this.xSpeed = 0;
        this.ySpeed = scale * -1;
        break;
      case "Down":
        this.xSpeed = 0;
        this.ySpeed = scale * 1;
        break;
      case "Left":
        this.xSpeed = scale * -1;
        this.ySpeed = 0;
        break;
      case "Right":
        this.xSpeed = scale * 1;
        this.ySpeed = 0;
        break;
    }
  };

  this.eat = function (fruit) {
    if (this.x === fruit.x && this.y === fruit.y) {
      this.total++;
      return true;
    } else {
      return false;
    }
  };

  this.reset = function () {
    this.pickLocation();
    this.xSpeed = 0;
    this.ySpeed = 0;
    this.total = 0;
    this.tail = [];
  };
}
