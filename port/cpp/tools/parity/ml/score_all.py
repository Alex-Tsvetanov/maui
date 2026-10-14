import json,os,sys,torch,numpy as np
from PIL import Image
sys.path.insert(0,'/workspace/maui/port/cpp/tools/parity/lib'); import pixel_score as P
torch.set_num_threads(6)
dino=torch.hub.load('facebookresearch/dinov2','dinov2_vits14',verbose=False).eval()
def prep(p,crop):
  im=Image.open(os.path.join(P.COMP,p)).convert('RGB'); im=im.crop((0,crop,im.width,im.height)); return im.resize((512,int(im.height*512/im.width)))
def df(im):
  x=torch.from_numpy(np.asarray(im.resize((224,448)),np.float32)/255).permute(2,0,1)[None]
  x=(x-torch.tensor([.485,.456,.406])[:,None,None])/torch.tensor([.229,.224,.225])[:,None,None]
  return torch.nn.functional.normalize(dino.forward_features(x)['x_norm_patchtokens'][0],dim=-1)
out=[]
for p in json.load(open(P.JSON)):
  for pl,v in p['platforms'].items():
    sc=v['screenshots']
    for t in ['light','dark']:
      a,b=P.full_res(sc.get('maui',{}).get(t)),P.full_res(sc.get('cpp',{}).get(t))
      if not(a and b and os.path.isfile(os.path.join(P.COMP,a)) and os.path.isfile(os.path.join(P.COMP,b))): continue
      crop=140 if pl=='android' else 0; A,B=prep(a,crop),prep(b,crop)
      if A.size!=B.size: B=B.resize(A.size)
      s=P.score_images(A,B,0)
      with torch.no_grad(): d=1-(df(A)*df(B)).sum(-1)
      out.append(dict(id=f"{p['name']}/{pl}/{t}",maui=a,cpp=b,ssim=s['ssim'],diff=s['diff_pct'],dino99=torch.quantile(d,.99).item()))
      print(len(out),flush=True)
json.dump(out,open('/workspace/labeler/allscores.json','w'))
