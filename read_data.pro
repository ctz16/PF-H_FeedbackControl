; IDL> .r read_data to execute

; Rout, Zout

shot = 165073

mdsopen, 'tst2', shot

; Saddle-loops
nso = 6
tag = 'magnetics.saddle_loops.signals:so' $
      + string(indgen(nso) + 1, format='(i0)')
x = mdsvalue('dim_of(' + tag[0] + ')')
y_so = fltarr([n_elements(x), n_elements(tag)])
for i = 0, n_elements(tag) - 1 do begin
  y_so[*, i] = mdsvalue(tag[i])
end
tfoff_so = mdsvalue('magnetics.saddle_loops:pickup_bt')
z1_so = mdsvalue('magnetics.saddle_loops:z1')
z2_so = mdsvalue('magnetics.saddle_loops:z2')

; Flux-loops
chname = ['o1', 'o2']
tag = 'magnetics.flux_loops.signals:' + chname
y_o = fltarr([n_elements(x), n_elements(tag)])
tfoff_o = fltarr(n_elements(tag))
z_o = fltarr(n_elements(tag))
r_o = fltarr(n_elements(tag))
tfoff = mdsvalue('magnetics.flux_loops:pickup_bt')
rall = mdsvalue('magnetics.flux_loops:r')
zall = mdsvalue('magnetics.flux_loops:z')
chall = mdsvalue('magnetics.flux_loops:signalname')
for i = 0, n_elements(tag) - 1 do begin
  y_o[*, i] = mdsvalue(tag[i])
  ich = (where(chall eq strupcase(chname[i])))[0]
  tfoff_o[i] = tfoff[ich]
  z_o[i] = zall[ich]
  r_o[i] = rall[ich]
end
chname = ['c6', 'c8']
tag = 'magnetics.flux_loops.signals:' + chname
y_c = fltarr([n_elements(x), n_elements(tag)])
tfoff_c = fltarr(n_elements(tag))
r_c = fltarr(n_elements(tag))
tfoff = mdsvalue('magnetics.flux_loops:pickup_bt')
rall = mdsvalue('magnetics.flux_loops:r')
chall = mdsvalue('magnetics.flux_loops:signalname')
for i = 0, n_elements(tag) - 1 do begin
  y_c[*, i] = mdsvalue(tag[i])
  ich = (where(chall eq strupcase(chname[i])))[0]
  tfoff_c[i] = tfoff[ich]
  r_c[i] = rall[ich]
end

; Magnetic probes
chname = 'op6'
tag = 'magnetics.bp_coils.signals:' + chname
y_op = mdsvalue(tag)
tfoff = mdsvalue('magnetics.bp_coils:pickup_bt')
rall = mdsvalue('magnetics.bp_coils:r')
chall = mdsvalue('magnetics.bp_coils:signalname')
ich = (where(chall eq strupcase(chname)))[0]
tfoff_op = tfoff[ich]
r_op = rall[ich]

; Bt
tag = 'magnetics.tf_coil:bt'
y_bt = mdsvalue(tag)
c_bt = mdsvalue('magnetics.tf_coil:tf:coef')
c_bt *= 200d-9 * mdsvalue('magnetics.tf_coil:turns')
c_bt /= mdsvalue('magnetics.tf_coil:r0')
y_bt_raw = mdsvalue('raw_of(magnetics.tf_coil:tf)')

mdsclose, 'tst2', shot

; Rout
y_out = y_o[*, 0] + total(y_so[*, 0:2], 2)
tangent = y_op * 2.0 * !PI * r_o[0]
delta_r = (y_c[*, 0] - y_out) / tangent
r_out = (r_c[0] + r_o[0] + delta_r) / 2.0
; Zout
psi = [[y_o[*, 0]], [y_so], [y_o[*, 1]]]
for i = 1, nso do begin
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
ploti, x, r_out, isec=isec++, /noupdate
ploti, x, z_out, isec=isec++, /noupdate
setregion, ymauto=2

end
