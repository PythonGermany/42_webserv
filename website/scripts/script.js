const body = document.body;
const mobileHeaderNav = "mobile-nav-display";
const darkMode = "dark-mode";
let lastScroll = 0;

window.addEventListener("scroll", () =>
{
	const currentScroll = window.scrollY;
	if (currentScroll > lastScroll && body.classList.contains(mobileHeaderNav))
		body.classList.remove(mobileHeaderNav);
	lastScroll = currentScroll;
});

function toggleMobileMenu()
{
	if (!body.classList.contains(mobileHeaderNav))
		body.classList.add(mobileHeaderNav);
	else
		body.classList.remove(mobileHeaderNav);
}

function getCookie(cookiename)
{
	var cookiestring = RegExp(cookiename + "=[^;]+").exec(document.cookie);
	return decodeURIComponent(!!cookiestring ? cookiestring.toString().replace(/^[^=]+./, "") : "");
}

function toggleDarkMode()
{
	if (!body.classList.contains(darkMode))
	{
		body.classList.add(darkMode);
		body.style.backgroundColor = "black";
		document.cookie = "dark-mode-enabled=1; expires=Fri, 31 Dec 9999 23:59:59 GMT; SameSite=Strict";
	}
	else
	{
		body.classList.remove(darkMode);
		body.style.backgroundColor = "#D9D8D2";
		document.cookie = "dark-mode-enabled=0; expires=Fri, 31 Dec 9999 23:59:59 GMT; SameSite=Strict";
	}
}

function checkCookies()
{
	if (getCookie("dark-mode-enabled") == "1")
		body.classList.add(darkMode);
	else
	{
		body.style.backgroundColor = "#D9D8D2";
		const collection = document.getElementsByClassName("dark-mode-input");
		for (let i = 0; i < collection.length; i++) {
			collection[i].checked = true;
		}
	}
}

window.onload = checkCookies();