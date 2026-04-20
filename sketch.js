// object to store water-related parameters
let water = { help: [true, 6], save: "data" };

// variable to link BGM to
let bgm;

// variable to store hydration percentage
let hydration;

// setup
async function setup() {
	// request save data
	let data = requestData("data");

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

	// disable music checkbox if BGM playback disabled
	if (!data.bgmEnabled)
		document.getElementById("music").checked = false;

	// initialize game
	initGame("setup");
}

// draw loop
function draw() {
	// calculate round hydration
	hydration = round(water.level);

	// execute if sufficiently and properly hydrated
	if (hydration >= 0 && hydration <= 100) {
		// display the cursor as hand if over the canvas
		cursor(HAND);

		// render Gerald dependent on water drinking
		if (water.cooldown.timer > frameCount)
			image(water.image.drink, 0, 0);
		else image(water.image.idle, 0, 0);

		// store current score
		water.score = round(frameCount / 60 - water.scoreOffset, 1);

		// display score
		fill(0);
		textAlign(LEFT, BASELINE);
		text("score: " + water.score + "\nhighscore: " + water.highscore, 5, 5 + textSize());

		// display help text before first click
		if (water.help[1]) {
			fill("#0AE" + round(15 * (water.help[1] / 6)).toString(16));
			text("\n\n\n\u{1F5B0} click to drink", 5, 5 + textSize());
			if (!water.help[0]) {
				water.help[1]--;
			}
		}

		// display hydration status
		// hydration in %
		textAlign(CENTER);
		fill(0);
		text("hydration: " + hydration + "%", 55, height - 12);
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
		// request save data
		let data = requestData("data");

		fill(255);
		textAlign(CENTER, CENTER);

		// show game over text
		if (hydration < 0) {
			// insufficient hydration message
			background("#d12");
			text("you perished of\n  dehydration...\n\n\n\n", width / 2, height / 2);
		}
		else if (hydration > 100) {
			// overhydration message
			background("#E26");
			text("you overhydrated\nand drowned!\n\n\n\n", width / 2, height / 2);
		}
		textStyle(BOLD);
		text("\n\n\ngame over\n\n", width / 2, height / 2);

		// copy score if it's a highscore
		if (water.score > water.highscore)
			data.highscore = water.score,
				water.highscore = water.score;

		// delete any save data if exists
		if (Object.hasOwn(data, "hydration")) delete data.hydration;
		if (Object.hasOwn(data, "score")) delete data.score;

		// store the new data object
		saveCookie(water.save, encodeData(data));

		// display score and highscore
		textStyle(NORMAL);
		text("\n\n\n\nscore: " + water.score + "\nhighscore: " + water.highscore, width / 2, height / 2);

		// stop looping and music
		noLoop();
		bgm.pause();
	}
}

// function that runs on canvas click
function gameClicked() {
	// only execute if sufficiently and properly hydrated
	if (water.level >= 0 && water.level <= 100 && water.cooldown.timer <= frameCount) {
		// do the following if water drinking cooldown is up

		// play sound if true
		water.sound.time(0);
		water.sound.play();

		// set cooldown
		water.cooldown.timer = frameCount + water.cooldown.value;

		// hydrate
		water.level += water.bottle;

		// fade help text on first click
		if (water.help[0]) {
			water.help[0] = false;
		}
	}
}

// game save function
function saveGame(method) {
	// request save data
	let data = requestData("data");

	// execute if game saving enabled
	if (method != "autosave" || document.getElementById("autosave").checked) {
		// update autosave state
		data.autosaveEnabled || (data.autosaveEnabled = true);

		// copy the data properties
		data.hydration = water.level,
			data.score = water.score;
	}
	// execute if game saving disabled
	else {
		// update autosave state
		data.autosaveEnabled && (data.autosaveEnabled = false);
	}

	// store the new data object
	saveCookie(water.save, encodeData(data));
}

// function for BGM playback
function testBgmPlayback() {
	// request save data
	let data = requestData("data");

	// execute if music playback enabled
	if (document.getElementById("music").checked) {
		// play BGM
		bgm.loop();

		// update bgm state
		data.bgmEnabled || (data.bgmEnabled = true);
	}
	// execute if music playback disabled
	else {
		// stop BGM
		bgm.stop();

		// update bgm state
		data.bgmEnabled && (data.bgmEnabled = false);
	}

	// store the new data object
	saveCookie(water.save, encodeData(data));
}

// function that runs on game reset
function initGame(method) {
	// request save data
	let data = requestData("data");

	// init highscore
	data.highscore
		&& (water.highscore = data.highscore)
		|| (water.highscore = 0);

	// store HTML page inputs to temporary variables
	let bottleInput = document.getElementById("bottleIn").value,
		speedInput = document.getElementById("speedIn").value,
		cooldownInput = document.getElementById("cooldownIn").value;

	// check for a value for bottle capacity...
	if (bottleInput)
		// ...and store to configuration if exists...
		water.bottle = bottleInput;
	// ...otherwise store the default
	else water.bottle = 30;

	// repeat for dehydration speed
	if (speedInput) water.speed = 1 / speedInput;
	else water.speed = 1 / 90;

	// repeat for drinking cooldown
	if (cooldownInput) water.cooldown = { value: cooldownInput, timer: 0 };
	else water.cooldown = { value: round(2.28 * getTargetFrameRate()), timer: 0 };

	// execute on setup if game saving enabled
	if (method == "setup" && (data.autosaveEnabled || data.autosaveEnabled === undefined)) {
		// enable autosave checkbox on HTML page
		document.getElementById("autosave").checked = true;

		// load water level from save data, or init new data
		water.level !== undefined
			&& (water.level = data.hydration)
			|| (water.level = 50);

		// same with score
		water.scoreOffset !== undefined
			&& (water.scoreOffset = -data.score)
			|| (water.scoreOffset = frameCount / 60);
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

	// store the new data object
	saveCookie(water.save, encodeData(data));

	// reset water drinking sound
	water.sound.stop();

	// attempt BGM playback
	testBgmPlayback();

	// reinit game loop
	loop();
}

// data encoding function
function encodeData(obj) {
	return btoa(JSON.stringify(obj));
}

function requestData(name) {
	// request cookie, create data object
	let cookie = loadCookie(name), data = {};
	// decode data if cookie valid
	cookie !== undefined
		&& (data = JSON.parse(atob(cookie)));
	// return data object
	return data;
}

// function for save data migration
// will most likely be retired in a month paralleling when old save data cookies will expire
function migrateSave(debug) {
	// set cookie mappings
	let cookies = [
		["wds-score", "score"],
		["wds-highscore", "highscore"],
		["wds-hydration", "hydration"],
		["wds-music", "bgmEnabled"],
		["wds-autosave", "autosaveEnabled"]
	],
		// create an object for new data
		data = {};

	// move any existing cookies to the data object
	for (let i in cookies) {
		let value = loadCookie(cookies[i][0]);
		if (value !== undefined) {
			data[cookies[i][1]] = value;
			deleteCookie(cookies[i][0]);
		}
	}

	// update property types
	data.bgmEnabled == 1 && (data.bgmEnabled = true);
	data.bgmEnabled == 0 && (data.bgmEnabled = false);
	data.autosaveEnabled
		&& (data.autosaveEnabled = true)
		|| (data.autosaveEnabled = false);
	for (let i in ["score", "highscore", "hydration"])
		data[i] && (data[i] = +data[i]);

	// save the new data
	saveCookie(water.save, encodeData(data));

	// return data if debugging
	if (debug) return data;
	// reload the page if not debugging
	else window.location.reload();
}
