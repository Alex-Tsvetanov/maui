import json,random,math
random.seed(1)
S=json.load(open('allscores.json'))
known={'scroll_to_group/android/dark','switch/android/dark','check_box/android/dark','ios_blur_effect/android/dark'}
for s in S:
  # p(DIFFERENT) heuristic: logistic on DINO p99 (tau 0.12) and SSIM (0.98)
  z=0.6*(s['dino99']-0.12)/0.04+0.4*(0.98-s['ssim'])/0.01
  s['p']=1/(1+math.exp(-z)); s['uncert']=abs(s['p']-0.5)
  s['prelabel']='SAME' if s['p']<0.3 else 'DIFFERENT' if s['p']>0.7 else 'MINOR'
S.sort(key=lambda s:s['p']); n=len(S)
lo,mid,hi=S[:int(n*.6)],S[int(n*.6):int(n*.9)],S[int(n*.9):]
pick={s['id']:s for s in S if s['id'] in known}
for grp,k in [(hi,110),(mid,110),(lo,80)]:
  for s in random.sample(grp,min(k,len(grp))):
    if len(pick)<300: pick[s['id']]=s
P=sorted(pick.values(),key=lambda s:s['uncert'])
json.dump([{k:s[k] for k in('id','maui','cpp','ssim','diff','dino99','p','prelabel')} for s in P],open('pairs.json','w'),indent=0)
print(len(P),sum(s['prelabel']=='MINOR' for s in P),[s['id'] for s in P if s['id'] in known])
