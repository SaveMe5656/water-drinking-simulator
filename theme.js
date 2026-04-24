const themeSelector = document.getElementById("theme");

let theme = {
  default: "--page-bg:light-dark(#bef, #023);--page-color:light-dark(#356, #def);--outline:#8886;--outline:light-dark(#09e, #09e);--background:#8883;--background:light-dark(#09e8, #09e8);--link:light-dark(#9de, #9de);--link-visited:light-dark(#ecf, #ecf)",
  basic: "--outline:#8886;--background:#8883;",
  light: [
    "light",
    "--page-bg:#bef;--page-color:#356;--outline:#09e;--background:#09e8;--link:#9de;--link-visited:#ecf;",
  ],
  dark: [
    "dark",
    "--page-bg:#023;--page-color:#def;--outline:#09e;--background:#09e8;--link:#9de;--link-visited:#ecf;",
  ],
};

function setTheme(style) {
  style || (style = themeSelector.value);
  let mode = "light dark",
    css = theme[style];
  if (Array.isArray(theme[style])) {
    mode = theme[style][0];
    css = theme[style][1];
  }
  document.querySelector("meta.theme").content = mode;
  document.querySelector("style.theme").innerHTML = ":root{" + css + "}";
  saveCookie("theme", style);
}

const style = loadCookie("theme");
setTheme(style);
themeSelector.value = style;
