
FUNC(int64_t) perf_impl(Py callable, Py args, Py kws, uint32_t const t) {
    int64_t tt, s, e;
    Py temp;
    uint32_t i = t;
    s = system_clock();
    while(i--) {
        if(!(temp=PyObject_Call(callable, args, kws)))
            return -1;
        Py_DECREF(temp);
    }
    e = system_clock();
    tt = e - s;
    return tt;
}
FUNC(Py) call_perf(Py *m, Py args) {

    int64_t tt, elapsed, calls;
    double data[50] = {0.0};
    double tmed, med;
    uint32_t nt;
    
    DEFINE_VARARGS(profile, 1, 'o', 'o', 'o') = {"func", "fargs", "kws"};
    VARARG(PyObject *, func, NULL);
    VARARG(PyObject *, fargs, NULL);
    VARARG(PyObject *, kws, NULL);    
    PARSE_VARARGS(); 

    if(!func)
        return PyTypeError("requires a callable");
    if(!fargs)
        fargs = EMPTY_TUPLE;
    if(!PyTuple_CheckExact(fargs))
        return PyTypeError("args must be a tuple"); 
    nt = 1;
    // find number of loops that take at least 0.05s to execute
    while(1) {
        tt = perf_impl(func, fargs, kws, nt);
        if(tt<0)
            return NULL;     
        if(tt >= 50000000LL)
            break;
        nt += 1 + nt * 2 / 3;
    }
    #define BILLION 1000000000.0
    elapsed = 0;
    calls = 0;
    size_t i = 0;

    // do loops until 0.5s have elapsed or 50 loops were completed
    while(elapsed < 500000000LL && i<50) {
        tt = perf_impl(func, fargs, kws, nt);
        if(tt < 0)
            return NULL;
        data[i] = (double)tt;
        elapsed += tt;
        ++i;
    }
    Py x = NULL, res = __list__(0), res2=NULL;
    if(!res)
        return NULL;
    for(size_t j =0; j<i; ++j) {
        x = __float__(data[j]);
        if(!x) goto 
            fail;
        if(PyList_Append(res, x)) 
            goto fail;
        Py_DECREF(x);
    }
    // get median calls per second
    if(PyList_Sort(res))
        goto fail;
    med = PyFloat_AS_DOUBLE(LIST_ITEMS(res)[Py_SIZE(res)/2]);
    med /= (double)nt;
    tmed = BILLION / med;
    res2 = __float__(tmed);
    Py_DECREF(res);
    return res2;
  fail:
    Py_XDECREF(res);
    Py_XDECREF(x);
    return NULL;
}