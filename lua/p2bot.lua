local http = require("socket.http")
local ltn12 = require("ltn12")
local io = require("io")
local util = require("util")

server = "yutori7.2ch.net/news4vip"

http.USERAGENT = "Mozilla/5.0 (compatible; p2bot/0.1; +http://p2m.giox.org/)"
-- http.USERAGENT = "Mozilla/5.0"

function http_get(url, headers)
    local body = {}
    local r, s, h = http.request {
        method = "GET",
        url = url,
        sink = ltn12.sink.table(body),
        headers = headers,
        redirect = true
    }
    return table.concat(body)
end

function get_dats()
    local subject = http_get("http://" .. server .. "/subject.txt", { referer = "http://www.google.co.jp/" }) 
    local dats = {}
    for k, v in string.gmatch(subject, "(.-).dat<>(.-)\n") do
        table.insert(dats, { id = k, url = "http://bg20.2ch.net/test/r.so/" .. server .. "/" .. k .. "/", name = v })
        -- dats[k] = { url = server .. "/dat/" .. k .. ".dat", name = v };
    end
    return dats;
end

function get_images(dat)
    local html = http_get(dat['url'])
    for image in string.gmatch(html, "//([%w%d%%%.-^~=?/_:]+%.jpg)") do
        print(image)
        util.usleep(200000)
    end
end

dats = get_dats()
for k, v in pairs(dats) do
    get_images(v)
    collectgarbage("collect")
end
