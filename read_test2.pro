function read_test2, file
  npoints = 100l
  data = 0

  openr, lun, file, /get_lun
  while (~eof(lun)) do begin
    s = ''
    readf, lun, s
    if (s eq 'probe') then readf, lun, s
    x = dblarr(npoints)
    readf, lun, x

    if (~keyword_set(data)) then begin
      data = create_struct(s, x)
    end else begin
      data = create_struct(data, s, x)
    end
  end

  free_lun, lun
  return, data
end
