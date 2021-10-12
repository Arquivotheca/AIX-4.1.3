/* */
  parm = 'hooks.arg'
  desc = 'hooks.dsc'
  data = 'hooks.dat'
  out  = 'hooks.doc'

  desc. = '***'
  parm. = '***'

  tab = '09'x

    do i = 1 by 1 while lines(desc)
      l = linein(desc)
      parse var l h d
      desc.h = d
    end

  call lineout desc

    do i = 1 by 1 while lines(parm)
      l = linein(parm)
      parse var l p 33 d
      p = space(p)
      parm.p = d
    end

  call lineout parm

    do while lines(data)
      l = linein(data)
      parse var l type '09'x file '09'x line '09'x h '09'x p1 '09'x p2 '09'x p3
      l = type      || '09'x ||,
          h         || '09'x ||,
          desc.h    || '09'x ||,
          parm.p1   || '09'x ||,
          parm.p2   || '09'x ||,
          parm.p3   || '09'x ||,
          file      || '09'x ||,
          line
      call lineout out,l
    end
