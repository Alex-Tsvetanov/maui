import json,os,sys,re,numpy as np,torch,lpips
from PIL import Image,ImageFilter,ImageSequence
from scipy import ndimage
sys.path.insert(0,'/workspace/maui/port/cpp/tools/parity/lib'); import pixel_score as P
from rapidocr_onnxruntime import RapidOCR
torch.set_num_threads(6)
C=P.COMP; L=json.load(open('labels.json')); pairs={p['id']:p for p in json.load(open('pairs.json'))}
old={r['id']:r for r in json.load(open('eval_rows.json'))} if os.path.exists('eval_rows.json') else {}
cache=json.load(open('v2_rows.json')) if os.path.exists('v2_rows.json') else {}
LP=lpips.LPIPS(net='alex',spatial=True,verbose=False); dino=torch.hub.load('facebookresearch/dinov2','dinov2_vits14',verbose=False).eval(); ocr=RapidOCR()
def load(p,crop): im=Image.open(os.path.join(C,p)).convert('RGB'); return im.crop((0,crop,im.width,im.height))
def motion_mask(path,crop,size):
  g=os.path.join(C,path[:-4]+'.gif')
  if not os.path.exists(g): return None
  fr=[np.asarray(f.convert('L').resize((size[0],int(size[0]*f.height/f.width))),float) for f in ImageSequence.Iterator(Image.open(g))]
  if len(fr)<2: return None
  v=np.std(np.stack(fr),0)>8; v=ndimage.binary_dilation(v,iterations=6)
  m=Image.fromarray((v*255).astype(np.uint8)).resize((size[0],size[1]+int(crop*size[0]/1000)))  # approx
  m=np.asarray(m.resize(size))>0; return m
def words(im):
  r,_=ocr(np.asarray(im)); W,H=im.size; out=[]
  for box,t,_c in (r or []):
    xs=[q[0] for q in box]; ys=[q[1] for q in box]; cx,cy=(min(xs)+max(xs))/2/W,(min(ys)+max(ys))/2/H
    for w in re.findall(r'\w+',t.lower()): out.append((w,cx,cy,(min(xs)/W,min(ys)/H,max(xs)/W,max(ys)/H)))
  return out
def ocr_diff(a,b):
  ub=list(range(len(b))); moved=0; missing=[]
  for w,x,y,bx in a:
    c=[j for j in ub if b[j][0]==w]
    if not c: missing.append((w,x,y,bx)); continue
    j=min(c,key=lambda j:(b[j][1]-x)**2+(b[j][2]-y)**2); ub.remove(j)
    if ((b[j][1]-x)**2+(b[j][2]-y)**2)**.5>0.02: moved+=1
  extra=[b[j] for j in ub]
  changed=sum(1 for m in missing if any(abs(e[1]-m[1])<.05 and abs(e[2]-m[2])<.02 for e in extra))
  return len(missing),len(extra),moved,changed,len(a)
def df(im):
  x=torch.from_numpy(np.asarray(im.resize((224,448)),np.float32)/255).permute(2,0,1)[None]
  x=(x-torch.tensor([.485,.456,.406])[:,None,None])/torch.tensor([.229,.224,.225])[:,None,None]
  return torch.nn.functional.normalize(dino.forward_features(x)['x_norm_patchtokens'][0],dim=-1)
for k in L:
  if k in cache: continue
  p=pairs[k]; crop=140 if '/android/' in k else 0
  A0,B0=load(p['maui'],crop),load(p['cpp'],crop)
  if A0.size!=B0.size: B0=B0.resize(A0.size)
  sz=(512,int(A0.height*512/A0.width)); A,B=A0.resize(sz),B0.resize(sz)
  mm=motion_mask(p['maui'],crop,sz); mc=motion_mask(p['cpp'],crop,sz)
  mask=None if mm is None and mc is None else ((mm if mm is not None else 0)|(mc if mc is not None else 0)).astype(bool)
  keep=np.ones(sz[::-1],bool) if mask is None else ~mask
  with torch.no_grad():
    lm=LP(torch.from_numpy(np.asarray(A,np.float32)/127.5-1).permute(2,0,1)[None],torch.from_numpy(np.asarray(B,np.float32)/127.5-1).permute(2,0,1)[None])[0,0].numpy()
    dp=(1-(df(A)*df(B)).sum(-1)).reshape(32,16).numpy()
  lm=lm*keep
  t=32; tiles=[lm[y:y+t,x:x+t].mean() for y in range(0,lm.shape[0]-t+1,t//2) for x in range(0,lm.shape[1]-t+1,t//2)]
  dmask=np.asarray(Image.fromarray(keep.astype(np.uint8)*255).resize((16,32)))>127
  # AA-tolerant element diff: blur both then threshold, connected components
  ga=np.asarray(A.convert('L').filter(ImageFilter.GaussianBlur(1.5)),float); gb=np.asarray(B.convert('L').filter(ImageFilter.GaussianBlur(1.5)),float)
  dm=(np.abs(ga-gb)>40)&keep; dm=ndimage.binary_opening(dm,iterations=1); lab,n=ndimage.label(dm)
  areas=np.bincount(lab.ravel())[1:] if n else np.array([0]); big=areas[areas>=40]
  oa,ob=words(A0),words(B0); miss,extra,moved,changed,na=ocr_diff(oa,ob)
  cache[k]=dict(lpips_tile_max=float(max(tiles)),lpips_tile_p95=float(np.percentile(tiles,95)),dino_patch_max_masked=float((dp*dmask).max()),
    elem_count=int(len(big)),elem_max_area=float(big.max()/dm.size*100 if len(big) else 0),elem_total=float(big.sum()/dm.size*100 if len(big) else 0),
    ocr_missing=miss,ocr_extra=extra,ocr_moved=moved,ocr_changed=changed,ocr_words=na,ocr_unmatched=miss+extra,
    motion_masked=float(0 if mask is None else mask.mean()))
  json.dump(cache,open('v2_rows.json','w')); print(len(cache),k,flush=True)
