require 'uri'
require 'socket'

class HTTP
  def get(url, header)
    uri = URI.parse(url)
    s = TCPSocket.new(uri.host, uri.port)
    s.write("GET #{uri.path} HTTP/1.1\r\n")
    s.write("Host: #{uri.host}\r\n")
    s.write("Useer-Agent: Mozilla/5.0\r\n")
    s.write("Connection: close\r\n")
    # "Mozilla/5.0 (compatible; p2bot/0.1; +http://p2m.giox.org/)"
    # Refere
    s.write("\r\n")

    return nil unless s.gets() =~ /(\d{3})/
    status = $1
    header = read_header(s)
    yield(status.to_i, header, s)
    s.close
  end

  def download(url, header, outfile)
    uri = URI.parse(url)
    s = TCPSocket.new(uri.host, uri.port)
    s.write("GET #{uri.path} HTTP/1.1\r\n")
    s.write("Host: #{uri.host}\r\n")
    s.write("Useer-Agent: Mozilla/5.0\r\n")
    s.write("Connection: close\r\n")
    # "Mozilla/5.0 (compatible; p2bot/0.1; +http://p2m.giox.org/)"
    # Refere
    s.write("\r\n")

    return nil unless s.gets() =~ /(\d{3})/
    return nil unless $1 == "200"
    header = read_header(s)
    open(outfile, "w") do |f|
      while (buf = s.read(1024))
        f.write(buf)  
      end
    end
    s.close
  rescue
    p url
  end

  def read_header(socket)
    header = {}
    while true
      line = socket.gets()
      break if line == "\r\n"
      h = line.split(':')
      header[h[0].downcase!] = h[1].strip!
    end
    return header
  end
end

subject = "http://yutori7.2ch.net/news4vip/subject.txt"

dat_list = []
HTTP.new.get(subject, nil) do |status, header, io|
  while (line = io.gets)
    dat = line.split('<>')
    dat_list.push(dat)
  end
end

dat_list.each do |e|
  HTTP.new.get("http://yutori7.2ch.net/news4vip/dat/#{e[0]}", nil) do |status, header, io|
    id = e[0].split('.')[0]
    puts id
    n = 0
    while (line = io.gets)
      line.scan(%r{//\S+?\.jpg}) do |url|
        HTTP.new.download("http:#{url}", nil, "images/#{id}_#{n}.jpg")
        n += 1
        sleep(0.5)
      end
    end
  end
  GC.start
  sleep(1)
end
