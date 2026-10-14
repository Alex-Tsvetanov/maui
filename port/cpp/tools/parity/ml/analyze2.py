import json,numpy as np
from sklearn.linear_model import LogisticRegression
from sklearn.preprocessing import StandardScaler
from sklearn.pipeline import make_pipeline
from sklearn.model_selection import LeaveOneOut,cross_val_predict
from sklearn.metrics import roc_auc_score
Lb=json.load(open('labels.json'))
R=json.load(open('eval_rows.json')); V=json.load(open('v2_rows.json')); rng=np.random.default_rng(0)
R=[dict(r,**V[r['id']],label=Lb[r['id']]['label']) for r in R if r['id'] in V and r['id'] in Lb]
base={'ssim':lambda r:1-r['ssim'],'diff':lambda r:r['diff'],'shift':lambda r:1-r['shift_ssim'],'lpips':lambda r:r['lpips'],'dmean':lambda r:r['dino_mean'],'dp99':lambda r:r['dino_p99'],'ds':lambda r:r['dreamsim'],'ocrj':lambda r:1-r['ocr_jacc']}
new={k:(lambda k:lambda r:np.log1p(r[k]))(k) for k in ['lpips_tile_max','lpips_tile_p95','dino_patch_max_masked','elem_count','elem_max_area','elem_total','ocr_missing','ocr_extra','ocr_moved','ocr_changed','ocr_unmatched']}
new['ocr_unmatched_frac']=lambda r:r['ocr_unmatched']/max(1,r['ocr_words'])
def boot(y,s,n=1000):
  a=[];i=np.arange(len(y))
  for _ in range(n):
    b=rng.choice(i,len(i))
    if len(set(y[b]))==2:a.append(roc_auc_score(y[b],s[b]))
  return np.percentile(a,[2.5,97.5])
def loo(X,y,C): return cross_val_predict(make_pipeline(StandardScaler(),LogisticRegression(C=C,max_iter=2000)),X,y,cv=LeaveOneOut(),method='predict_proba')[:,1]
old14=set(w[0] for w in json.load(open('misclassified.json')))
y=np.array([r['label']=='DIFFERENT' for r in R]).astype(int); print(f"n={len(y)} pos={y.sum()} (MINOR->SAME)")
Xb=np.array([[f(r) for f in base.values()] for r in R]); Xn=np.array([[f(r) for f in new.values()] for r in R])
for n,X in {'v1 baseline':Xb,'v2 new only':Xn,'v1+v2':np.hstack([Xb,Xn])}.items():
  p=loo(X,y,1.0); lo,hi=boot(y,p); print(f"| {n} | {roc_auc_score(y,p):.3f} | {lo:.2f}-{hi:.2f} | {((p>=.5)==y).mean():.2f} |",flush=True)
  if n=='v2 new only':
    wr={R[i]['id']:(R[i]['label'],round(float(p[i]),2)) for i in range(len(R)) if (p[i]>=.5)!=y[i]}
    json.dump(wr,open('v2_wrong.json','w'),indent=1)
    print('OLD14 fixed:',sorted(old14-set(wr))); print('OLD14 still wrong:',sorted(old14&set(wr))); print('NEW errors:',{k:v for k,v in wr.items() if k not in old14})
m=make_pipeline(StandardScaler(),LogisticRegression(C=1.0,max_iter=2000)).fit(Xn,y)
print('coefs',dict(zip(new,np.round(m[-1].coef_[0],2))))
