require("socket.http")
require("ltn12")
require("io")

function debug_table(table)
    for k, v in pairs(table) do
        print(k .. ':' .. v)
    end
end

function p2db_get(id)
    local data = {}
    local f = io.open("db/" .. id .. ".dat", "r")
    if (not f) then
        return nil
    end

    local s = f:read()
    local id, name, num, modified, length = string.match(s, "(.+)<>(.+)<>(.+)<>(.+)<>(.+)")
    data['id'] = id
    data['name'] = name
    data['num'] = num
    data['modified'] = modified
    data['length'] = length
    f:close()
    return data
end

function p2db_set(id, name, num, modified, length)
    local f = io.open("db/" .. id .. ".dat", "w")
    f:write(string.format("%s<>%s<>%s<>%s<>%s", id, name, num, modified, length))
    f:close()
end

p2db = {}
function p2db.new()
    return {
        get = p2db_get,
        set = p2db_set
    }
end

function http_get(url, headers)
    local body = {}
    local r, s, h = socket.http.request {
        method = "GET",
        url = url,
        sink = ltn12.sink.table(body),
        headers = headers,
        redirect = true
    }
    return table.concat(body), s, h
end

function get_dats()
    local subject = http_get("http://" .. server .. "/subject.txt", { referer = "http://www.google.co.jp/" }) 
    local dats = {}
    for k, v in string.gmatch(subject, "(.-).dat<>(.-)\n") do
        table.insert(dats, { id = k, url = "http://" .. server .. "/dat/" .. k .. ".dat", name = v });
    end
    return dats;
end

-- pl-loader
function download_image(url, outfile)
    print(url)
    --local data, status, header = http_get("http://" .. url)
    --if (status == 200 and header['content-type'] == 'image/jpeg') then
    --    local f = io.open(outfile, "w")
    --    f:write(data)
    --    f:close()
    --end
end

-- If-Modified-Since: Fri, 23 Oct 2009 09:37:06 GMT
-- Range: bytes= 86641-
-- 206 Partial Content
-- 304 Not Modified
-- 203 dat oti
function get_thread(dat)
    local data = db.get(dat['id'])
    local header = {}
    local num = 0

    if data then
        num = data['num']
        header['If-Modified-Since'] = data['modified']
        header['Range'] = "bytes= " .. data['length'] .. "-"
    end
    local html, status, header = http_get(dat['url'], header)
    if (status == 200 or status == 206) then
        for image in string.gmatch(html, "//([%w%d%%%.-^~=?/_:]+%.jpg)") do
            download_image(image, 'images/' .. dat['id'] .. '-' .. num ..'.jpg')
            num = num + 1
            usleep(500000)
        end
        local length = header['content-length']
        if data then
            length = length + data['length']
        end
        db.set(dat['id'], dat['name'], num, header['last-modified'], length)
    end
end

server = "yutori7.2ch.net/news4vip"
socket.http.USERAGENT = "Mozilla/5.0 (compatible; p2bot/0.1; +http://p2m.giox.org/)"
-- http.USERAGENT = "Mozilla/5.0"
db = p2db.new()
dats = get_dats()
for k, v in pairs(dats) do
    get_thread(v)
    collectgarbage("collect")
    sleep(1)
end
