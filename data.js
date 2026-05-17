
// data requesting function
function requestData(name) {
	const cookie = loadCookie(name); let data;
	try { data = JSON.parse(atob(cookie)); }
	catch (error) {
		console.warn("Invalid data string. Using new save data...");
		data = { version: [], save: {}, volume: {} };
	}
	return data;
}

// data encoding function
function encodeData(obj) {
	return btoa(JSON.stringify(obj));
}

// data import function
function importData(b64Data, noReload) {
	// default to HTML input
	b64Data || (b64Data = document.getElementById('data').value);

	// attempt parsing data to ensure validity
	try { JSON.parse(atob(b64Data)); }
	// abort if invalid
	catch {
		// log warning
		console.warn("Attempted to load invalid data. Aborting...");

		return;
	}

	// store the new save data
	saveCookie(water.save, b64Data);

	// return data if debugging
	if (noReload) return b64Data;
	// reload the page otherwise
	else window.location.reload();
}

// data export function
function exportData(debug) {
	// request save data
	let data = requestData(water.save);

	// copy current game properties
	data.save.hydration = water.hydration.real,
		data.save.score = water.score.value;

	// output to HTML if not debugging
	debug || (document.getElementById('data').value = encodeData(data));

	// return data object
	return data;
}

// save data fixing function
function repairData(noReload) {
	// request save data
	let data = requestData(water.save);

	// (1.5.2)
	// move all autosave data to one property
	if (data.autosaveEnabled) {
		data.autosave = { enabled: data.autosaveEnabled };
		delete data.autosaveEnabled;
	}
	for (let i in ["highscore", "score", "hydration"])
		if (data[i]) {
			data.autosave[i] = data[i];
			delete data[i];
		}

	// (v1.6.1)
	// move existing autosave data to be standard save data
	if (data.autosave) {
		data.save = data.autosave || {};
		delete data.autosave;
		// rename autosave toggle value
		if (data.save.enabled) {
			data.save.autosave = data.save.enabled;
			delete data.save.enabled;
		}
	}

	// save the new data
	saveCookie(water.save, encodeData(data));

	// return data if debugging
	if (noReload) return data;
	// reload the page if not debugging
	else window.location.reload();
}
