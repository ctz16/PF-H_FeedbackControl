; IDL> .r rzout to execute

; Rout, Zout

; shot = 145000
shot = 165527

mdsopen, 'tst2', shot

; Saddle-loops
tag = 'magnetics.saddle_loops.signals:so' + string(indgen(6) + 1, format='(i0)')
x = mdsvalue('dim_of(' + tag[0] + ')')
y_so = fltarr([n_elements(x), n_elements(tag)])
c_so = fltarr(n_elements(tag))
for i = 0, n_elements(tag) - 1 do begin
  y_so[*, i] = mdsvalue(tag[i])
  c_so[i] = mdsvalue(tag[i] + ':coef')
end
z1_so = mdsvalue('magnetics.saddle_loops:z1')
z2_so = mdsvalue('magnetics.saddle_loops:z2')

; Flux-loops
tag = 'magnetics.flux_loops.signals:' + ['o1', 'o2']
y_o = fltarr([n_elements(x), n_elements(tag)])
c_o = fltarr(n_elements(tag))
for i = 0, n_elements(tag) - 1 do begin
  y_o[*, i] = mdsvalue(tag[i])
  c_o[i] = mdsvalue(tag[i] + ':coef')
end
z_o = [0.355, -0.355]
r_o = 0.699
tag = 'magnetics.flux_loops.signals:' + ['c6', 'c8']
y_c = fltarr([n_elements(x), n_elements(tag)])
c_c = fltarr(n_elements(tag))
for i = 0, n_elements(tag) - 1 do begin
  y_c[*, i] = mdsvalue(tag[i])
  c_c[i] = mdsvalue(tag[i] + ':coef')
end
r_c = 0.120

; Magnetic probes
tag = 'magnetics.bp_coils.signals:op6'
y_op = mdsvalue(tag)
c_op = mdsvalue(tag + ':coef')
r_op = 0.677

mdsclose, 'tst2', shot

; Rout
y_out = y_o[*, 0] + total(y_so[*, 0:2], 2)
tangent = y_op * 2.0 * !PI * r_o
delta_r = (y_c[*, 0] - y_out) / tangent
r_out = (r_c + r_o + delta_r) / 2.0
; Zout
psi = [[y_o[*, 0]], [y_so], [y_o[*, 1]]]
for i = 1, n_elements(c_so) do begin
  psi[*, i] += psi[*, i - 1]
end
z = [z_o[0], z1_so, z_o[1]]
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
ploti, x, r_out, ydefrange=[0.1, 0.6], isec=isec++, /noupdate
ploti, x, z_out, ydefrange=[-0.2, 0.2], isec=isec++, /noupdate
setregion, ymauto=2
end
