import json,numpy as np,collections
from sklearn.linear_model import LogisticRegression
from sklearn.preprocessing import StandardScaler
from sklearn.pipeline import make_pipeline
from sklearn.model_selection import LeaveOneOut,cross_val_predict
from sklearn.metrics import roc_auc_score
exec(open('analyze2.py').read().split("def boot")[0].split("Lb=")[0])
Lb=json.load(open('labels.json')); orig=set(json.load(open('orig76.json')))
E0=json.load(open('eval_rows.json')); V=json.load(open('v2_rows.json')); rng=np.random.default_rng(0)
R=[dict(r,**V[r['id']],label=Lb[r['id']]['label'],note=Lb[r['id']]['note']) for r in E0 if r['id'] in V and r['id'] in Lb]
src=open('analyze2.py').read(); exec(src[src.index("base="):src.index("def boot")])
def boot(y,s,n=2000):
  a=[];i=np.arange(len(y))
  for _ in range(n):
    b=rng.choice(i,len(i))
    if len(set(y[b]))==2:a.append(roc_auc_score(y[b],s[b]))
  return np.percentile(a,[2.5,97.5])
M=lambda:make_pipeline(StandardScaler(),LogisticRegression(C=1.0,max_iter=2000))
y=np.array([r['label']=='DIFFERENT' for r in R]).astype(int)
Xb=np.array([[f(r) for f in base.values()] for r in R]); Xn=np.array([[f(r) for f in new.values()] for r in R])
tr=np.array([r['id'] in orig for r in R]); te=~tr
print(f"total {len(R)}; train {tr.sum()} (pos {y[tr].sum()}); test {te.sum()} (pos {y[te].sum()}, labels {dict(collections.Counter(r['label'] for r,t in zip(R,te) if t))})")
for n,X in [('v1 baseline',Xb),('v2',Xn)]:
  p=M().fit(X[tr],y[tr]).predict_proba(X[te])[:,1]; lo,hi=boot(y[te],p)
  print(f"HELD-OUT {n}: AUC {roc_auc_score(y[te],p):.3f} ({lo:.2f}-{hi:.2f}) acc {((p>=.5)==y[te]).mean():.2f}")
  if n=='v2':
    ids=[r for r,t in zip(R,te) if t]
    for r,pp,yy in zip(ids,p,y[te]):
      if (pp>=.5)!=yy: print('  HO-ERR',r['id'],r['label'],round(float(pp),2),r['note'][:120])
for n,X in [('v1 baseline',Xb),('v2',Xn),('v1+v2',np.hstack([Xb,Xn]))]:
  p=cross_val_predict(M(),X,y,cv=LeaveOneOut(),method='predict_proba')[:,1]; lo,hi=boot(y,p)
  print(f"LOO all {n}: AUC {roc_auc_score(y,p):.3f} ({lo:.2f}-{hi:.2f}) acc {((p>=.5)==y).mean():.2f} errors {int(((p>=.5)!=y).sum())}")
