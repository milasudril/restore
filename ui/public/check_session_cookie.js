"use strict";

async function redirect_if_not_logged_in(loc, doc)
{
	let response = await send_request("/login");

	if(!response.succeeded)
	{	return; }

	if(response.message.session_status !== "logged_in")
	{
		doc.cookie = "redirected_from=" + loc.href + "; SameSite=Strict; path=/;"
		loc.href = "/ui/public/login.html";
	}
}
