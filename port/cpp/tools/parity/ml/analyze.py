import json,numpy as np
from sklearn.linear_model import LogisticRegression
from sklearn.preprocessing import StandardScaler
from sklearn.pipeline import make_pipeline
from sklearn.model_selection import LeaveOneOut,cross_val_predict
from sklearn.metrics import roc_auc_score
R=json.load(open('eval_rows.json')); rng=np.random.default_rng(0)
# orient: higher = more different
F={'1-SSIM':lambda r:1-r['ssim'],'diff%':lambda r:r['diff'],'1-shiftSSIM':lambda r:1-r['shift_ssim'],'LPIPS':lambda r:r['lpips'],
   'DINO mean':lambda r:r['dino_mean'],'DINO p99':lambda r:r['dino_p99'],'DreamSim':lambda r:r['dreamsim'],'1-OCR jaccard':lambda r:1-r['ocr_jacc']}
def boot(y,s,n=2000):
  a=[];i=np.arange(len(y))
  for _ in range(n):
    b=rng.choice(i,len(i))
    if len(set(y[b]))==2: a.append(roc_auc_score(y[b],s[b]))
  return np.percentile(a,[2.5,97.5])
def thr(y,s):
  best=(0,0)
  for t in np.unique(s): acc=((s>=t)==y).mean(); best=max(best,(acc,t))
  return best
out=[]
for mode,pos in [('MINOR->SAME',{'DIFFERENT'}),('MINOR->DIFFERENT',{'DIFFERENT','MINOR'})]:
  y=np.array([r['label'] in pos for r in R]).astype(int)
  out.append(f"\n### {mode}: n={len(y)}, positives={y.sum()}\n| metric | AUC | 95% CI | best thr acc |\n|---|---|---|---|")
  X=np.array([[f(r) for f in F.values()] for r in R])
  for j,(n,f) in enumerate(F.items()):
    s=X[:,j]; lo,hi=boot(y,s); acc,t=thr(y,s); out.append(f"| {n} | {roc_auc_score(y,s):.3f} | {lo:.2f}-{hi:.2f} | {acc:.2f} @ {t:.4g} |")
  for name,cols in [('LR all',list(range(len(F)))),('LR DINO p99+OCR+diff',[1,5,7])]:
    m=make_pipeline(StandardScaler(),LogisticRegression(C=1.0,max_iter=1000))
    p=cross_val_predict(m,X[:,cols],y,cv=LeaveOneOut(),method='predict_proba')[:,1]
    lo,hi=boot(y,p); out.append(f"| {name} (LOO-CV) | {roc_auc_score(y,p):.3f} | {lo:.2f}-{hi:.2f} | {((p>=.5)==y).mean():.2f} @ 0.5 |")
    if mode=='MINOR->SAME' and name=='LR all':
      wrong=[(R[i]['id'],R[i]['label'],round(float(p[i]),2),R[i]['note']) for i in range(len(R)) if (p[i]>=.5)!=y[i]]
      json.dump(wrong,open('misclassified.json','w'),indent=1)
print("\n".join(out))
