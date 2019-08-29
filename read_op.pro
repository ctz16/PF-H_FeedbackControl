shot = 165545;current_shot()
d = read_test('test_' + strtrim(shot, 2) + '.txt')
t = d.time*1e-6
ch = 'OP6'

mdsopen, 'tst2', shot
tf = mdsvalue('/bt')
x_tf = mdsvalue('dim_of(\bt)')

y = mdsvalue('.magnetics.bp_coils.signals:' + ch)
x = mdsvalue('dim_of(.magnetics.bp_coils.signals:' + ch + ')')
i = str_locate(mdsvaluem('.magnetics.bp_coils:signalname'), ch)
mdsclose, 'tst2', shot

tag = '.bp_coils:pickup_bt[' + strtrim(i, 2) + ']'
y_cor = correct_pickup(x, y, shot, tag, '')

isec = 0
ploti, x, smooth([[y], [y_cor]], [100, 1])
oploti, t, d.p8
end
