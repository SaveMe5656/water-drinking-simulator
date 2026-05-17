// object to store/get game values
let water = {
	// game version
	version: "v1.6.3",
	// score object
	score: {
		// function to return score/highscore string
		string() {
			// generate string to inform score
			let string = "score: " + (water.score.value ? water.score.value.toFixed(1) : 0);
			// append highscore if exists
			water.score.highscore && (string += "\nhighscore: " + water.score.highscore.toFixed(1));

			// return string
			return string;
		}
	},
	// game tick object
	tick: {
		last: 0
	},
	// help text controller
	help: [true, 6],
	// save data cookie name
	save: "data"
};

// setup
async function setup() {
	// request save data
	let data = requestData(water.save);

	// init audio
	water.audio = {
		sound: {
			drink: createAudio("assets/bloxy-cola.mp3")
		},
		music: {
			bgm: createAudio("assets/Sneaky%20Snitch.mp3")
		}
	};

	// adjust BGM volume
	water.audio.music.bgm.volume(0.5);

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
	data.bgmEnabled || (document.getElementById("bgm").checked = false);

	// set font to match page
	textFont(getComputedStyle(document.documentElement).fontFamily);

	// initialize game
	initGame("setup");
}

// draw loop
function draw() {
	// store current tick timestamp
	water.tick.current = millis() / 1e3 * getTargetFrameRate();

	// calculate game tick modifier
	water.tick.modifier = water.tick.current - water.tick.last;

	// calculate round hydration
	water.hydration.mean = round(water.hydration.real);

	// execute if sufficiently and properly hydrated
	if (water.hydration.mean >= 0 && water.hydration.mean <= 100) {
		// display the cursor as hand if over the canvas
		cursor(HAND);

		// render Gerald dependent on water drinking
		if (water.cooldown.timer > 0) {
			image(water.image.drink, 0, 0)
			// decrement cooldown variable
			water.cooldown.timer -= water.tick.modifier / getTargetFrameRate();
		}
		else image(water.image.idle, 0, 0);

		// store current score
		water.score.value += water.tick.modifier / getTargetFrameRate();

		// display score
		fill(0);
		textAlign(LEFT, BASELINE);
		text(water.score.string(), 5, 5 + textSize());

		// display help text before first click
		if (water.help[1] > 0) {
			// height offset value
			let heightOffset = ((2 + !!water.score.highscore) * textLeading())
			// help text
			image(water.image.mouseHint, 5, 5 + heightOffset, textLeading(), textLeading());
			fill("#0AE" + round(15 * (water.help[1] / 6)).toString(16));
			text(" click to drink", 5 + textLeading(), (5 + textSize()) + heightOffset);
			// first click fade out
			water.help[0] || (water.help[1] -= water.tick.modifier);
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

		// decrement hydration
		water.hydration.real -= water.speed / water.tick.modifier / getTargetFrameRate() * 60;

		// store last tick timestamp
		water.tick.last = water.tick.current;
	}
	// execute if unsufficiently or improperly hydrated
	else {
		// request save data
		let data = requestData(water.save);

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
		water.score.value > water.score.highscore && (
			data.highscore = water.score.value,
			water.score.highscore = data.highscore
		);

		// delete any save data if exists
		Object.hasOwn(data.save, "hydration") && delete data.save.hydration;
		Object.hasOwn(data.save, "score") && delete data.save.score;

		// store the new data object
		saveCookie(water.save, encodeData(data));

		// display score and highscore
		textStyle(NORMAL);
		text("\n\n\n\n" + water.score.string(), width / 2, height / 2);

		// stop looping and music
		noLoop();
		water.audio.music.bgm.pause();
	}
}

// function that runs on game reset
function initGame(method) {
	// request save data
	let data = requestData(water.save);

	// execute on setup
	if (method == "setup") {
		// display game version on HTML page
		document.getElementById("version").innerHTML = water.version;

		// execute if data version doesn't match game version
		if (!data.version.includes(water.version)) {
			// attempt to fix save data 
			data = repairData(1);

			// save new game versions to cookies
			data.version.push(water.version);
		}

		// set game volume
		data.volume = {
			sound: changeVolume(data.volume.sound),
			music: changeVolume("music", data.volume.music)
		};

		// update HTML volume sliders
		document.getElementById("sound").value = data.volume.sound,
			document.getElementById("music").value = data.volume.music;

		// enable autosave checkbox on HTML page if enabled
		(data.save.autosave || data.save.autosave === undefined) && (document.getElementById("autosave").checked = true);

		// init hydration
		water.hydration = {
			real: data.save.hydration || 50
		};

		// init score
		water.score.value = data.save.score || 0;
	}
	// execute otherwise
	else {
		// init hydration and score
		water.hydration = { real: 50 };
		water.score.value = 0;
	}

	// init autosave timer
	clearInterval(water.autosave);
	water.autosave = setInterval(() => { saveGame("autosave") }, 6e4);

	// init score incrementing timer

	// init highscore
	water.score.highscore = data.highscore || 0;

	// store HTML page inputs to temporary variables
	let bottleInput = document.getElementById("bottleIn").value,
		speedInput = document.getElementById("speedIn").value,
		cooldownInput = document.getElementById("cooldownIn").value;

	// init bottle capacity
	water.bottle = bottleInput || 30;

	// init dehydration speed
	water.speed = speedInput ? 1 / speedInput : 1 / 90;

	// reset drinking cooldown
	water.cooldown = {
		timer: 0,
		value: cooldownInput || 2.28
	};

	// store the new data object
	saveCookie(water.save, encodeData(data));

	// reset water drinking sound
	water.audio.sound.drink.stop();

	// attempt BGM playback
	testBgmPlayback();

	// reinit game loop
	loop();
}

// function that runs on canvas click
function gameClicked() {
	// only execute if sufficiently and properly hydrated
	if (water.hydration.real >= 0 && water.hydration.real <= 100 && water.cooldown.timer <= frameCount) {
		// do the following if water drinking cooldown is up

		// play sound if true
		water.audio.sound.drink.time(0);
		water.audio.sound.drink.play();

		// set cooldown
		water.cooldown.timer = water.cooldown.value;

		// hydrate
		water.hydration.real += water.bottle;

		// begin help text fade on first click
		water.help[0] && (water.help[0] = false);
	}
}

// function for BGM playback
function testBgmPlayback() {
	// request save data
	let data = requestData(water.save);

	// execute if music playback enabled
	if (document.getElementById("bgm").checked) {
		// play BGM
		water.audio.music.bgm.loop();

		// update bgm state
		data.bgmEnabled || (data.bgmEnabled = true);
	}
	// execute if music playback disabled
	else {
		// stop BGM
		water.audio.music.bgm.stop();

		// update bgm state
		data.bgmEnabled && (data.bgmEnabled = false);
	}

	// store the new data object
	saveCookie(water.save, encodeData(data));
}

// volume changing function
function changeVolume(type, volume) {
	// default to changing sound volume from document input
	switch (typeof type) {
		case "string": break;
		case "number":
			volume = type;
		default:
			type = "sound";
	}
	volume || (volume = +document.getElementById(type).value);

	// change volume of each audio element of the specified type
	for (let i in water.audio[type])
		water.audio[type][i].volume(volume);

	// request save data
	let data = requestData(water.save);

	// copy the respective volume value
	data.volume[type] = volume;

	// store the new data object
	saveCookie(water.save, encodeData(data));

	// return volume value for debugging purposes
	return volume;
}

// game save function
function saveGame(method) {
	// request save data
	let data = requestData(water.save);

	// execute if saving normally or autosave enabled
	if (method != "autosave" || document.getElementById("autosave").checked) {
		// update autosave state
		data.save.autosave || (data.save.autosave = true);

		// copy the data properties
		data.save.hydration = water.hydration.real,
			data.save.score = water.score.value;
	}
	// execute otherwise
	else {
		// update autosave state
		data.save.autosave == false || (data.save.autosave = false);
	}

	// store the new data object
	saveCookie(water.save, encodeData(data));
}
