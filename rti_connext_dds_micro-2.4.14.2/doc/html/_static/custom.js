function getHost() {
    hostname = window.location.host;
    if(hostname == "") {
	hostname = "localhost:8080";
    }
    return hostname;
}
