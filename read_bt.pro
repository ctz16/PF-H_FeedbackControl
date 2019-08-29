shot = 165554;current_shot()
d = read_test('test_' + strtrim(shot, 2) + '.txt')
t = d.time*1e-6
;t = (30+1.3*findgen(n_elements(d.p9)))*1e-3
mdsopen, 'tst2', shot
y = mdsvalue('\bt')
x = mdsvalue('dim_of(\bt)')
mdsclose, 'tst2', shot
ploti, x, y
oploti, t, d.p9
end
