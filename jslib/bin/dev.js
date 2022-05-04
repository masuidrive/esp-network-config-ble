var http = require('http');
var fs = require('fs');
var path = require('path');

let PORT = process.env['PORT'] ? parseInt(process.env['PORT'], 10) : 3000;
for(let i = 0; i < process.argv.length; ++i) {
  if(["-p", "--port"].includes(process.argv[i])) {
    PORT = parseInt(process.argv[++i], 10);
  }
}

function escapeRegExp(string) {
  return string.replace(/[.*+?^${}()|[\]\\]/g, '\\$&'); // $& means the whole matched string
}

http.createServer(function (request, response) {
  console.log('request ', request.url);
  const routes = [
    [ "/dist", "./dist" ],
    [ "/", "./example" ],
  ];

  let urlPath = path.normalize(decodeURI(request.url.replace(/\?.*/, "")));
  const route = routes.find((r) => urlPath.startsWith(r[0]));
  if(!route) {
    response.writeHead(500, { 'Content-Type': 'text/html' });
    response.end("Not configured", 'utf-8');
  }
  let filePath = urlPath.replace(new RegExp(`^${escapeRegExp(route[0])}`), `${route[1]}/`);
  filePath = filePath.replaceAll(/\/+/g, "/");

  try {
    var stats = fs.statSync(filePath);
    if (stats.isDirectory()) {
      filePath += "/index.html";
    }
  } catch(e) {
    fs.readFile('./404.html', function(error, content) {
      if(error) content = "Not found";
      response.writeHead(404, { 'Content-Type': 'text/html' });
      response.end(content, 'utf-8');
    });
    return;
  }

  var extname = String(path.extname(filePath)).toLowerCase();
  const mimeTypes = {
    '.html': 'text/html',
    '.js': 'text/javascript',
    '.jsx': 'text/javascript',
    '.css': 'text/css',
    '.json': 'application/json',
    '.png': 'image/png',
    '.jpg': 'image/jpg',
    '.gif': 'image/gif',
    '.svg': 'image/svg+xml',
    '.wav': 'audio/wav',
    '.mp4': 'video/mp4',
    '.woff': 'application/font-woff',
    '.ttf': 'application/font-ttf',
    '.eot': 'application/vnd.ms-fontobject',
    '.otf': 'application/font-otf',
    '.wasm': 'application/wasm'
  };

  const contentType = mimeTypes[extname] || 'application/octet-stream';
  fs.readFile(filePath, function(error, content) {
    if (error) {
      response.writeHead(500);
      response.end('Sorry, check with the site admin for error: '+error.code+' ..\n');
    }
    else {
      response.writeHead(200, { 'Content-Type': contentType });
      response.end(content, 'utf-8');
    }
  });
}).listen(PORT);

console.info(`Server running at http://localhost:${PORT}/`);