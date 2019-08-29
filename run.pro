shot = 165410;current_shot()
d = read_test('test_' + strtrim(shot, 2) + '.txt')
t = d.time*1e-3-1.0

ztarget = 0.02
rtarget = 0.32

isec = 0
ploti, t, d.rout,ytitle='R[m]', 'b+-', isec=isec++, /noupdate
;plotl, rtarget, col='k', /hl, /noupdate
;ploti, t, 255-d.d_pf,ytitle='R duty circle', 'r+-', isec=isec++, /noupdate
ploti, t, d.zout,ytitle='z[m]', 'b+-', isec=isec++, /noupdate
;plotl, ztarget, col='k', /hl, /noupdate
;ploti, t, d.duty_z,ytitle='z duty cicle', 'r+-', isec=isec++, /noupdate
;plotl, 0.0, /hl, col='k', /noupdate
setregion, xdefrange=[10, 110], ymauto=2, /noupdate

d = get_rzout(shot=shot)
oploti, d.x*1e3, smooth(d.rout,100), 'b', isec=0, $
        ydefrange=[0.1, 0.4], /noupdate
oploti, d.x*1e3, smooth(d.zout,100), 'b', isec=1, $
        ydefrange=[-0.2, 0.2]

shot = 165409;current_shot()
d = read_test('test_' + strtrim(shot, 2) + '.txt')
t = d.time*1e-3-1.0

ztarget = 0.02
rtarget = 0.32

isec = 0
oploti, t, d.rout,ytitle='R[m]', 'r+-', isec=isec++, /noupdate
;plotl, rtarget, col='k', /hl, /noupdate
;ploti, t, 255-d.d_pf,ytitle='R duty circle', 'r+-', isec=isec++, /noupdate
oploti, t, d.zout,ytitle='z[m]', 'r+-', isec=isec++, /noupdate
;plotl, ztarget, col='k', /hl, /noupdate
;ploti, t, d.duty_z,ytitle='z duty cicle', 'r+-', isec=isec++, /noupdate
;plotl, 0.0, /hl, col='k', /noupdate
setregion, xdefrange=[10, 110], ymauto=2, /noupdate

d = get_rzout(shot=shot)
oploti, d.x*1e3, smooth(d.rout,100), 'r', isec=0, $
        ydefrange=[0.1, 0.4], /noupdate
oploti, d.x*1e3, smooth(d.zout,100), 'r', isec=1, $
        ydefrange=[-0.2, 0.2]


shot = 165408;current_shot()
d = read_test('test_' + strtrim(shot, 2) + '.txt')
t = d.time*1e-3-1.0

ztarget = 0.02
rtarget = 0.32

isec = 0
oploti, t, d.rout,ytitle='R[m]', 'k+-', isec=isec++, /noupdate
;plotl, rtarget, col='k', /hl, /noupdate
;ploti, t, 255-d.d_pf,ytitle='R duty circle', 'r+-', isec=isec++, /noupdate
oploti, t, d.zout,ytitle='z[m]', 'k+-', isec=isec++, /noupdate
;plotl, ztarget, col='k', /hl, /noupdate
;ploti, t, d.duty_z,ytitle='z duty cicle', 'r+-', isec=isec++, /noupdate
;plotl, 0.0, /hl, col='k', /noupdate
setregion, xdefrange=[10, 110], ymauto=2, /noupdate

d = get_rzout(shot=shot)
oploti, d.x*1e3, smooth(d.rout,100), 'k', isec=0, $
        ydefrange=[0.1, 0.4], /noupdate
oploti, d.x*1e3, smooth(d.zout,100), 'k', isec=1, $
        ydefrange=[-0.2, 0.2]

end

;shot = 164970

;ch = ['o1', 'so' + strtrim(indgen(6) + 1, 2), 'c6']
;n = n_elements(ch)
;y = dblarr(100, n)
;for i = 0, n - 1 do begin
;  d = read_ascii(ch[i] + '_' + strtrim(shot, 2) + '.txt')
;  y[*, i] = d.field1
;end
;t = 11.0 + findgen(n_elements(rout))
;image, t, indgen(6) + 1, y[*, 1:6], int=0

;end
