shot = 165132;current_shot()
file = '/home/chang/workspace/'
file += 'test_' + strtrim(shot, 2) + '.txt'
d = read_test2(file)
;t = d.time*1e-3-1.0
t = 32.0 + findgen(n_elements(d.rout))*1.2

rtarget = 0.32
ztarget = 0.02

isec = 0
ploti, t, d.rout, 'r+-', isec=isec++, /noupdate
;plotl, rtarget, col='k', /hl, /noupdate
ploti, t, 255-d.d_pf, 'r+-', isec=isec++, /noupdate
ploti, t, d.zout, 'r+-', isec=isec++, /noupdate
plotl, ztarget, col='k', /hl, /noupdate
ploti, t, d.duty_z, 'r+-', isec=isec++, /noupdate
plotl, 0.0, /hl, col='k', /noupdate
setregion, xdefrange=[10, 110], ymauto=2, /noupdate

d = get_rzout(shot=shot)
oploti, d.x*1e3, smooth(d.rout,100), 'k', isec=0, $
        ydefrange=[0.1, 0.4], /noupdate
oploti, d.x*1e3, smooth(d.zout,100), 'k', isec=2, $
        ydefrange=[-0.2, 0.2]

end

shot = 165059
d = read_test('test_' + strtrim(shot, 2) + '.txt')
x = 14.0 + findgen(n_elements(d.rout))

mdsopen, 'tst2', shot

; Saddle-loops
y_so = [[d.p1-d.p1[0]], [d.p2-d.p2[0]], [d.p3-d.p3[0]], $
        [d.p4-d.p4[0]], [d.p5-d.p5[0]], [d.p6-d.p6[0]]]
z1_so = mdsvalue('magnetics.saddle_loops:z1')

; Flux-loops
y_o = d.p0-d.p0[0]
z_o = 0.355
r_o = 0.699
y_c = d.p7-d.p7[0]
r_c = 0.120

; Magnetic probes
y_op = d.p8-d.p8[0]
r_op = 0.677

mdsclose, 'tst2', shot

; Rout
y_out = y_o + total(y_so[*, 0:2], 2)
tangent = y_op * 2.0 * !PI * r_o
delta_r = (y_c - y_out) / tangent
r_out = (r_c + r_o + delta_r) / 2.0
; Zout
psi = [[y_o], [y_so]]
for i = 1, 6 do begin
  psi[*, i] += psi[*, i - 1]
end
z = [z_o, z1_so]
d = 3
c = dblarr([d, d], /nozero)
for i = 0, d - 1 do begin
  for j = i, d - 1 do begin
    c[i, j] = total(z^(i + j))
  end
  for j = 0, i - 1 do begin
    c[i, j] = c[j, i]
  end
end
zp = dblarr(n_elements(z), d, /nozero)
for i = 0, d - 1 do begin
  zp[*, i] = z^i
end
c = zp # invert(c)
a = psi # c
z_out = -a[*, 1] / (2.0 * a[*, 2])

isec = 0
ploti, x, y_so, ytitle='Saddle-loops', isec=isec++, /noupdate
ploti, x, [[y_o], [y_c]], ytitle='Flux-loops', isec=isec++, /noupdate
ploti, x, y_op, xtitle='time [s]', ytitle='Bp coils', isec=isec++, /noupdate
ploti, x, y_out, isec=isec++, /noupdate
ploti, x, tangent, isec=isec++, /noupdate
ploti, x, delta_r, isec=isec++, /noupdate
ploti, x, r_out, isec=isec++, /noupdate
ploti, x, z_out, isec=isec++, /noupdate
setregion, ymauto=2

end

shot = 164970

ch = ['o1', 'so' + strtrim(indgen(6) + 1, 2), 'c6']
n = n_elements(ch)
y = dblarr(100, n)
for i = 0, n - 1 do begin
  d = read_ascii(ch[i] + '_' + strtrim(shot, 2) + '.txt')
  y[*, i] = d.field1
end
t = 11.0 + findgen(n_elements(rout))
image, t, indgen(6) + 1, y[*, 1:6], int=0

end
