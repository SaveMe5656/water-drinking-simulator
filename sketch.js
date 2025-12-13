// object to store water-related parameters
let water = {
	cooldown: 2
};

// variable to link BGM to
let bgm;

// variable to store hydration percentage
let hydration;

// setup
async function setup() {
	// init water drinking sound
	water.sound = createAudio("assets/bloxy-cola.mp3");

	// init BGM
	bgm = await createAudio("assets/Sneaky%20Snitch.mp3");

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
	initGame("setup");

	// disable Music checkbox if BGM playback disabled/failed
	if (!+loadCookie("wds-music") || navigator.getAutoplayPolicy("mediaelement") != "allowed")
		document.getElementById("bgm").checked = false;
}

// draw loop
function draw() {
	hydration = round(water.level);
	// execute if sufficiently and properly hydrated
	if (hydration >= 0 && hydration <= 100) {
		// display the cursor as hand if over the canvas
		cursor(HAND);

		// check water drinking status...
		if (water.sound.time() > 0 && water.sound.time() < water.sound.duration() / 2)
			image(water.image.drink, 0, 0);
		else image(water.image.idle, 0, 0);

		// store current score
		water.score = round(frameCount / 60 - water.scoreOffset, 1);

		// display score
		fill(0);
		textAlign(LEFT);
		text("Score: " + water.score + "\nHighscore: " + water.highscore, 5, 5 + textSize());

		// display hydration status
		// hydration in %
		textAlign(CENTER);
		text("Hydration: " + hydration + "%", 55, height - 12);
		// hydration as progressbar
		fill(128);
		stroke(0);
		rect(4.5, height - 10.5, 101, 6, 3);
		if (water.level > 15) fill("#0AE");
		else fill("#E43");
		noStroke();
		rect(5, height - 10, water.level, 5, 2.5);

		// attempt autosave every minute
		if (Number.isInteger((frameCount + 1) / getTargetFrameRate() / 60))
			saveGame("autosave");

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

		// save score if it's a highscore
		if (water.score > water.highscore) {
			saveCookie("wds-highscore", water.score);
			water.highscore = water.score;
		}

		// delete any save data if exists
		if (loadCookie("wds-hydration")) deleteCookie("wds-hydration");
		if (loadCookie("wds-score")) deleteCookie("wds-score");

		// display GAME OVER and score/highscore
		text("\n\n\nGAME OVER\nScore: " + water.score + "\nHighscore: " + water.highscore, width / 2, (height * 3) / 7);

		// stop looping and music
		noLoop();
		bgm.pause();
	}
}

// function that runs on canvas click
function gameClicked() {
	// only execute if sufficiently and properly hydrated
	if (water.level >= 0 && water.level <= 100) {
		// do the following if water drinking cooldown is up
		if (water.sound.time() == 0 || water.sound.time() >= water.sound.duration() / 2) {
			// restart sound playback if true
			water.sound.time(0);

			// hydrate
			water.level += water.bottle;
		}

		// attempt to play sound
		water.sound.play();
	}
}

// game save function
function saveGame(method) {
	// execute if game saving enabled
	if (method != "autosave" || document.getElementById("autosave").checked) {
		// save state to cookies
		if (loadCookie("wds-autosave")) deleteCookie("wds-autosave");

		// perform save
		saveCookie("wds-hydration", water.level);
		saveCookie("wds-score", water.score);
	}
	// execute if game saving disabled
	else {
		// save state to cookies
		if (loadCookie("wds-autosave") != "disabled")
			saveCookie("wds-autosave", "disabled");
	}
}

// function for BGM playback
function testBgmPlayback() {
	let bgmState = +loadCookie("wds-music");
	// execute if music playback enabled
	if (document.getElementById("bgm").checked) {
		// play BGM
		bgm.loop();

		// save state to cookies
		if (!bgmState)
			saveCookie("wds-music", 1);
	}
	// execute if music playback disabled
	else {
		// stop BGM
		bgm.stop();

		// save state to cookies
		if (bgmState || isNaN(bgmState))
			saveCookie("wds-music", 0);
	}
}

// function that runs on game reset
function initGame(method) {
	// init highscore
	water.highscore = +loadCookie("wds-highscore");
	water.highscore || (water.highscore = 0);

	// store HTML page inputs to temporary variables
	let bottleInput = document.getElementById("bottleIn").value,
		speedInput = document.getElementById("speedIn").value;

	// check for a value for bottle capacity...
	if (bottleInput)
		// ...and store to configuration if exists...
		water.bottle = bottleInput;
	// ...otherwise store the default
	else water.bottle = 30;

	// do the same for dehydration speed
	if (speedInput) water.speed = 1 / speedInput;
	else water.speed = 1 / 90;

	// execute on setup if game saving enabled
	if (method == "setup" && !loadCookie("wds-autosave")) {
		// toggle checkbox on HTML page
		document.getElementById("autosave").checked = true;

		// attempt loading water level and score from cookies
		water.level = +loadCookie("wds-hydration");
		water.scoreOffset = -loadCookie("wds-score");
		// init default values if failed
		if (isNaN(water.level)) {
			water.level = 50;
			water.scoreOffset = frameCount / 60;
		}
	}
	// execute otherwise
	else {
		// reset water level and score
		water.level = 50;
		water.scoreOffset = frameCount / 60;

		// delete any save data if exists
		if (loadCookie("wds-hydration")) deleteCookie("wds-hydration");
		if (loadCookie("wds-score")) deleteCookie("wds-score");
	}

	// reset water drinking sound
	water.sound.stop();

	// attempt BGM playback
	testBgmPlayback();

	// reinit game loop
	loop();
}
