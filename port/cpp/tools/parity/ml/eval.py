import json,os,sys,re,numpy as np,torch,lpips
from PIL import Image
sys.path.insert(0,'/workspace/maui/port/cpp/tools/parity/lib'); import pixel_score as P
from dreamsim import dreamsim
from rapidocr_onnxruntime import RapidOCR
torch.set_num_threads(6)
L=json.load(open('labels.json')); pairs={p['id']:p for p in json.load(open('pairs.json'))}
LP=lpips.LPIPS(net='alex',verbose=False); dino=torch.hub.load('facebookresearch/dinov2','dinov2_vits14',verbose=False).eval()
ds,dsp=dreamsim(pretrained=True,device='cpu',cache_dir='/workspace/dreamsim_cache',dreamsim_type='open_clip_vitb32'); ocr=RapidOCR()
def load(p,crop): im=Image.open(os.path.join(P.COMP,p)).convert('RGB'); return im.crop((0,crop,im.width,im.height))
def tt(im): return torch.from_numpy(np.asarray(im,np.float32)/127.5-1).permute(2,0,1)[None]
def df(im):
  x=torch.from_numpy(np.asarray(im.resize((224,448)),np.float32)/255).permute(2,0,1)[None]
  x=(x-torch.tensor([.485,.456,.406])[:,None,None])/torch.tensor([.229,.224,.225])[:,None,None]
  return torch.nn.functional.normalize(dino.forward_features(x)['x_norm_patchtokens'][0],dim=-1)
def toks(im): r,_=ocr(np.asarray(im)); return set(w.lower() for t in (r or []) for w in re.findall(r'\w+',t[1]))
rows=json.load(open('eval_rows.json')); done={r['id'] for r in rows}
for k,v in L.items():
  if k in done: continue
  p=pairs[k]; crop=140 if '/android/' in k else 0
  A0,B0=load(p['maui'],crop),load(p['cpp'],crop)
  if A0.size!=B0.size: B0=B0.resize(A0.size)
  A=A0.resize((512,int(A0.height*512/A0.width))); B=B0.resize(A.size)
  s=P.score_images(A,B,0)
  a=np.asarray(A); best=s['ssim']
  for dx in range(-4,5,2):
    for dy in range(-4,5,2):
      if dx or dy:
        Bs=Image.fromarray(np.roll(np.asarray(B),(dy,dx),(0,1))); best=max(best,P.score_images(A,Bs,0)['ssim'])
  with torch.no_grad():
    lp=LP(tt(A),tt(B)).item(); d=1-(df(A)*df(B)).sum(-1); dsd=ds(dsp(A),dsp(B)).item()
  ta,tb=toks(A0),toks(B0); jac=len(ta&tb)/max(1,len(ta|tb))
  rows.append(dict(id=k,label=v['label'],note=v['note'],ssim=s['ssim'],diff=s['diff_pct'],shift_ssim=best,lpips=lp,dino_mean=d.mean().item(),dino_p99=torch.quantile(d,.99).item(),dreamsim=dsd,ocr_jacc=jac))
  print(len(rows),flush=True)
json.dump(rows,open('eval_rows.json','w'),indent=0)
