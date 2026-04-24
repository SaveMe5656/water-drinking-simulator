// object to store water-related parameters
let water = {
	version: "v1.5.0 debug",
	help: [true, 6],
	save: "data"
};

// variable to link BGM to
let bgm;

// setup
async function setup() {
	// request save data
	let data = requestData("data");

	// init water drinking sound
	water.sound = createAudio("assets/bloxy-cola.mp3");

	// init BGM
	bgm = await createAudio("assets/Sneaky%20Snitch.mp3");

	// init images
	water.image = {
		idle: await loadImage("assets/waterd2.png"),
		drink: await loadImage("assets/waterd33.png"),
		mouseHint: await loadImage("https://cdn.jsdelivr.net/gh/jdecked/twemoji@latest/assets/72x72/1f5b1.png")
	};

	// create and name the game canvas
	game = createCanvas(320, 302);
	game.id("gerald");

	// restrict canvas action to the canvas
	game.mouseClicked(gameClicked);

	// disable music checkbox if BGM playback disabled
	data.bgmEnabled || (document.getElementById("music").checked = false);

	// initialize game
	initGame("setup");
}

// draw loop
function draw() {
	// calculate round hydration
	water.hydration.mean = round(water.hydration.real);

	// execute if sufficiently and properly hydrated
	if (water.hydration.mean >= 0 && water.hydration.mean <= 100) {
		// display the cursor as hand if over the canvas
		cursor(HAND);

		// render Gerald dependent on water drinking
		water.cooldown.timer > frameCount
			? image(water.image.drink, 0, 0)
			: image(water.image.idle, 0, 0);

		// store current score
		water.score = round(frameCount / 60 - water.scoreOffset, 1);

		// display score
		fill(0);
		textAlign(LEFT, BASELINE);
		text("score: " + water.score + "\nhighscore: " + water.highscore, 5, 5 + textSize());

		// display help text before first click
		if (water.help[1]) {
			// help text
			image(
				water.image.mouseHint,
				5,
				5 + (3 * textLeading()),
				textLeading(),
				textLeading()
			);
			fill("#0AE" + round(15 * (water.help[1] / 6)).toString(16));
			text(
				" click to drink",
				5 + textLeading(),
				(5 + textSize()) + (3 * textLeading())
			);
			// first click fade out
			water.help[0] || water.help[1]--;
		}

		// display hydration status
		// hydration in %
		textAlign(CENTER);
		fill(0);
		text("hydration: " + water.hydration.mean + "%", 55, height - 12);
		// hydration as progressbar
		fill(128);
		stroke(0);
		rect(4.5, height - 10.5, 101, 6, 3);
		fill(water.hydration.real > 15 ? "#0AE" : "#E43");
		noStroke();
		rect(5, height - 10, water.hydration.real, 5, 2.5);

		// attempt autosave every minute
		Number.isInteger((frameCount + 1) / getTargetFrameRate() / 60) && saveGame("autosave");

		// decrement hydration
		water.hydration.real -= water.speed;
	}
	// execute if unsufficiently or improperly hydrated
	else {
		// request save data
		let data = requestData("data");

		fill(255);
		textAlign(CENTER, CENTER);

		// show game over text
		if (water.hydration.mean < 0) {
			// insufficient hydration message
			background("#d12");
			text("you perished of\n  dehydration...\n\n\n\n", width / 2, height / 2);
		}
		else if (water.hydration.mean > 100) {
			// overhydration message
			background("#E26");
			text("you overhydrated\nand drowned!\n\n\n\n", width / 2, height / 2);
		}
		textStyle(BOLD);
		text("\n\n\ngame over\n\n", width / 2, height / 2);

		// copy score if it's a highscore
		water.score > water.highscore && (
			data.highscore = water.score,
			water.highscore = water.score
		);

		// delete any save data if exists
		Object.hasOwn(data, "hydration") && delete data.hydration;
		Object.hasOwn(data, "score") && delete data.score;

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
	if (water.hydration.real >= 0 && water.hydration.real <= 100 && water.cooldown.timer <= frameCount) {
		// do the following if water drinking cooldown is up

		// play sound if true
		water.sound.time(0);
		water.sound.play();

		// set cooldown
		water.cooldown.timer = frameCount + water.cooldown.value;

		// hydrate
		water.hydration.real += water.bottle;

		// begin help text fade on first click
		water.help[0] && (water.help[0] = false);
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
		data.hydration = water.hydration.real,
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

	// execute on setup
	if (method == "setup") {
		// display game version on HTML page
		document.getElementById("version").innerHTML = water.version;

		// save new game versions to cookies
		Array.isArray(data.version)
			? data.version.at(-1).toString() == water.version.toString()
			|| data.version.push(water.version)
			: data.version = [water.version];
	}

	// init highscore
	water.highscore = data.highscore || 0;

	// store HTML page inputs to temporary variables
	let bottleInput = document.getElementById("bottleIn").value,
		speedInput = document.getElementById("speedIn").value,
		cooldownInput = document.getElementById("cooldownIn").value;

	// init bottle capacity
	water.bottle = bottleInput || 30;

	// init dehydration speed
	water.speed = speedInput ? 1 / speedInput : 1 / 90;

	// init drinking cooldown
	water.cooldown = {
		value: cooldownInput || round(2.28 * getTargetFrameRate()),
		timer: 0
	};

	// execute on setup if game saving enabled
	if (method == "setup" && (data.autosaveEnabled || data.autosaveEnabled === undefined)) {
		// enable autosave checkbox on HTML page
		document.getElementById("autosave").checked = true;

		// init hydration
		water.hydration = {
			real: data.hydration || 50
		};

		// init score
		water.scoreOffset = -data.score || frameCount / 60;
	}
	// execute otherwise
	else {
		// reset hydration and score
		water.hydration = { real: 50 };
		water.scoreOffset = frameCount / 60;

		// delete any save data if exists
		Object.hasOwn(data, "hydration") && delete data.hydration;
		Object.hasOwn(data, "score") && delete data.score;
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

// data requesting function
function requestData(name) {
	let cookie = loadCookie(name), data = {};
	cookie !== undefined && (data = JSON.parse(atob(cookie)));
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
	data.bgmEnabled = !!+data.bgmEnabled;
	data.autosaveEnabled = !!+data.autosaveEnabled;
	for (let i in ["highscore", "score", "hydration"])
		data[i] && (data[i] = +data[i]);

	// save the new data
	saveCookie(water.save, encodeData(data));

	// return data if debugging
	if (debug) return data;
	// reload the page if not debugging
	else window.location.reload();
}
