function read_test, file
  npoints = 100l
  data = 0

  openr, lun, file, /get_lun
  s = ''
  readf,lun,s
  while (s ne 'cnt')do begin
  readf,lun,s
  end
  readf,lun,npoints
  data = create_struct(s,npoints)
  while (~eof(lun)) do begin
    readf,lun,s
    if (s eq 'probe') then readf, lun, s
    x = dblarr(npoints)
    readf, lun,x
    data = create_struct(data, s, x)
  end

  free_lun, lun
  return, data
end
