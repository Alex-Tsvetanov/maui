import json,os,threading,mimetypes
from http.server import ThreadingHTTPServer,BaseHTTPRequestHandler
from urllib.parse import urlparse,parse_qs
D='/workspace/labeler'; COMP='/workspace/maui/port/cpp/docs/comparison'
TOKEN=open(D+'/token').read().strip(); LAB=D+'/labels.json'; lock=threading.Lock()
pairs=json.load(open(D+'/pairs.json'))
def labels(): return json.load(open(LAB)) if os.path.exists(LAB) else {}
class H(BaseHTTPRequestHandler):
  def ok(self):
    q=parse_qs(urlparse(self.path).query); 
    if q.get('t',[''])[0]!=TOKEN: self.send_error(403); return False
    return True
  def send(self,b,ct):
    self.send_response(200); self.send_header('Content-Type',ct); self.send_header('Cache-Control','no-store'); self.end_headers(); self.wfile.write(b)
  def do_GET(self):
    if not self.ok(): return
    u=urlparse(self.path).path
    if u=='/': self.send(open(D+'/index.html','rb').read(),'text/html; charset=utf-8')
    elif u=='/api/pairs': self.send(json.dumps({'pairs':pairs,'labels':labels()}).encode(),'application/json')
    elif u.startswith('/img/'):
      rel=os.path.normpath(u[5:]); p=os.path.join(COMP,rel)
      if '..' in rel or not os.path.isfile(p): self.send_error(404); return
      self.send(open(p,'rb').read(),mimetypes.guess_type(p)[0] or 'image/png')
    else: self.send_error(404)
  def do_POST(self):
    if not self.ok(): return
    b=json.loads(self.rfile.read(int(self.headers['Content-Length'])))
    if b.get('label') not in ('SAME','MINOR','DIFFERENT') or b.get('id') not in {p['id'] for p in pairs}: self.send_error(400); return
    with lock:
      L=labels(); import time; L[b['id']]={'label':b['label'],'note':str(b.get('note',''))[:1000],'ts':time.strftime('%Y-%m-%dT%H:%M:%S%z')}
      json.dump(L,open(LAB+'.tmp','w'),indent=1); os.replace(LAB+'.tmp',LAB)
    self.send(b'{"ok":true}','application/json')
  def log_message(self,*a): pass
ThreadingHTTPServer(('127.0.0.1',8787),H).serve_forever()
