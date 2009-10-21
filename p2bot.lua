local http = require("socket.http")
local ltn12 = require("ltn12")
local io = require("io")

local subject = "http://yutori7.2ch.net/news4vip/subject.txt"

http.USERAGENT = "Mozilla/5.0 (compatible; p2bot/0.1; +http://p2m.giox.org/)"
-- http.USERAGENT = "Mozilla/5.0"

r, s, h = http.request {
    method = "GET",
    url = subject,
    sink = ltn12.sink.file(io.stdout),
    headers = { referer = "http://www.google.co.jp/" },
    redirect = true
}
