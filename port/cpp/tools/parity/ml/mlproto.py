import json,os,random,sys,io,difflib,time
import numpy as np,torch,lpips
from PIL import Image,ImageDraw,ImageFilter
sys.path.insert(0,'maui/port/cpp/tools/parity/lib'); import pixel_score as P
from rapidocr_onnxruntime import RapidOCR
torch.set_num_threads(6); random.seed(0)
ocr=RapidOCR(); L=lpips.LPIPS(net='alex',verbose=False)
dino=torch.hub.load('facebookresearch/dinov2','dinov2_vits14',verbose=False).eval()
pages=json.load(open(P.JSON))
pairs=[]
for p in pages:
  for pl,v in p['platforms'].items():
    sc=v['screenshots']
    for t in ['light','dark']:
      a,b=P.full_res(sc.get('maui',{}).get(t)),P.full_res(sc.get('cpp',{}).get(t))
      if a and b and os.path.isfile(os.path.join(P.COMP,a)) and os.path.isfile(os.path.join(P.COMP,b)):
        pairs.append((f"{p['name']}/{pl}/{t}",a,b,pl))
known={'scroll_to_group/android/dark','switch/android/dark','check_box/android/dark','ios_blur_effect/android/dark'}
samp=[x for x in pairs if x[0] in known]+random.sample([x for x in pairs if x[0] not in known],96)
def load(p): return Image.open(os.path.join(P.COMP,p)).convert('RGB')
def prep(im,crop):
  im=im.crop((0,crop,im.width,im.height)); w=512; return im.resize((w,int(im.height*w/im.width)))
def tt(im,s=None):
  x=torch.from_numpy(np.asarray(im.resize(s or im.size),np.float32)/127.5-1).permute(2,0,1)[None];return x
def dfeat(im):
  x=torch.from_numpy(np.asarray(im.resize((224,448)),np.float32)/255).permute(2,0,1)[None]
  x=(x-torch.tensor([.485,.456,.406])[:,None,None])/torch.tensor([.229,.224,.225])[:,None,None]
  f=dino.forward_features(x)['x_norm_patchtokens'][0]; return torch.nn.functional.normalize(f,dim=-1)
def text(im):
  r,_=ocr(np.asarray(im)); return ' '.join(t[1] for t in (r or []))
def score(a,b):
  if a.size!=b.size: b=b.resize(a.size)
  s=P.score_images(a,b,0)
  with torch.no_grad():
    lp=L(tt(a),tt(b)).item(); fa,fb=dfeat(a),dfeat(b)
    dpatch=1-(fa*fb).sum(-1); d_mean=dpatch.mean().item(); d_max=torch.quantile(dpatch,.99).item()
  ta,tb=text(a),text(b); tm=difflib.SequenceMatcher(None,ta,tb).ratio() if (ta or tb) else 1.0
  return dict(ssim=s['ssim'],diff=s['diff_pct'],lpips=lp,dino=d_mean,dino99=d_max,text=tm)
def perturb_noise(im):
  k=random.choice(['shift','jpeg','gamma','blur'])
  if k=='shift': return Image.fromarray(np.roll(np.asarray(im),random.choice([2,3,4]),axis=random.choice([0,1]))),k
  if k=='jpeg': b=io.BytesIO(); im.save(b,'JPEG',quality=60); return Image.open(b).convert('RGB'),k
  if k=='gamma': return Image.fromarray(np.clip(np.asarray(im,float)*1.06,0,255).astype(np.uint8)),k
  return im.filter(ImageFilter.GaussianBlur(0.8)),k
def perturb_real(im):
  im=im.copy(); r,_=ocr(np.asarray(im)); d=ImageDraw.Draw(im); k=random.choice(['drop_text','recolor_box'])
  if r and k=='drop_text':
    box=random.choice(r)[0]; xs=[q[0] for q in box]; ys=[q[1] for q in box]
    bg=im.getpixel((max(0,int(min(xs))-3),int(min(ys))))
    d.rectangle([min(xs),min(ys),max(xs),max(ys)],fill=bg); return im,k
  x,y=random.randint(0,im.width-120),random.randint(0,im.height-60)
  d.rectangle([x,y,x+110,y+44],fill=(random.randint(0,255),0,random.randint(0,255))); return im,'recolor_box'
rows=[];t0=time.time()
for name,a,b,pl in samp:
  crop=140 if pl=='android' else 0
  A,B=prep(load(a),crop),prep(load(b),crop)
  rows.append(dict(name=name,kind='real_pair',label=1 if name in known else None,**score(A,B)))
  if len(rows)<=200:
    N,k=perturb_noise(A); rows.append(dict(name=name,kind='syn_noise:'+k,label=0,**score(A,N)))
    R,k=perturb_real(A); rows.append(dict(name=name,kind='syn_change:'+k,label=1,**score(A,R)))
  print(len(rows),round(time.time()-t0),flush=True)
json.dump(rows,open('ml_rows.json','w'),indent=0); print('secs',time.time()-t0)
