// object to store water-related parameters
let water = {};

// variable to store hydration percentage
let hydration;

// setup
async function setup() {
  // init water drinking sound
  water.sound = createAudio("assets/bloxy-cola.mp3");

  // init water drinking guy images
  water.image = {
    idle: await loadImage("assets/waterd2.png"),
    drink: await loadImage("assets/waterd33.png"),
  };

  // create the game canvas
  game = createCanvas(320, 302);

  // restrict canvas action to the canvas
  game.mouseClicked(gameClicked);

  // initialize game
  gameInit();
}

//console.log(document.getElementById('ID').innerHTML);

// draw loop
function draw() {
  hydration = round(water.level);
  // execute if sufficiently and properly hydrated
  if (hydration >= 0 && hydration <= 100) {
    // display the cursor as hand if over the canvas
    cursor(HAND);

    // check water drinking status...
    if (
      water.sound.time() > 0 &&
      water.sound.time() < water.sound.duration() / 2
    )
      image(water.image.drink, 0, 0);
    else image(water.image.idle, 0, 0);

    // store current score
    water.score = round(frameCount / 60 - water.scoreOffset, 1);

    // display score
    fill(0);
    textAlign(LEFT);
    text("Score: " + water.score, 5, 5 + textSize());

    // display hydration status
    // hydration in %
    textAlign(CENTER);
    text("Hydration: " + hydration + "%", 55, height - 12);
    // hydration as progressbar
    fill(128);
    stroke(0);
    rect(5, height - 10, 100, 5);
    if (water.level > 15) fill("#0AE");
    else fill("#E43");
    noStroke();
    rect(5, height - 10, water.level, 5);

    // decrement hydration level
    water.level -= water.speed;
  }
  // execute if unsufficiently or improperly hydrated
  else {
    fill(255);
    textAlign(CENTER);

    // execute if player had not sufficiently hydrated
    if (hydration < 0) {
      background("#d12");
      text("You perished of\n  dehydration...", width / 2, (height * 3) / 7);
    }
    // otherwise execute if player had overhydrated and drowned
    else if (hydration > 100) {
      background("#E26");
      text("You overhydrated\nand drowned!", width / 2, (height * 3) / 7);
    }
    text("\n\n\nGAME OVER\nScore: " + water.score, width / 2, (height * 3) / 7);
  }
}

// function that runs on canvas click
function gameClicked() {
  // only execute if sufficiently and properly hydrated
  if (water.level >= 0 && water.level <= 100) {
    // do the following if water drinking cooldown is up
    if (
      water.sound.time() == 0 ||
      water.sound.time() >= water.sound.duration() / 2
    ) {
      // restart sound playback if true
      water.sound.time(0);

      // hydrate
      water.level += water.bottle;
    }

    // attempt to play sound
    water.sound.play();
  }
}

// function that runs on game reset
function gameInit() {
  // store HTML page inputs to temporary variables
  let bottleInput = document.getElementById("bottleIn").value,
    speedInput = document.getElementById("speedIn").value;

  // check for a value for bottle capacity...
  if (bottleInput)
    // ...and store to configuration if exists...
    water.bottle = bottleInput;
  // ...otherwise store the default
  else water.bottle = 30;

  // do the same for dehydration speed...
  if (speedInput)
    // ...and store to configuration if exists...
    water.speed = 1 / speedInput;
  // ...otherwise store its default
  else water.speed = 1 / 90;

  // reset water level and score
  water.level = 50;
  water.scoreOffset = frameCount / 60;

  // reset water drinking sound
  water.sound.stop();
}
