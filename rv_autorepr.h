#ifndef RV_AUTOREPRS_H
#define RV_AUTOREPRS_H
#define lognormvariate_args_t normalvariate_args_t
#define normvariate_args_t normalvariate_args_t
#define uniformvariate_args_t uniform_args_t
FUNC(Py) randiter_beta_repr(randiter *ri) {

    Py result = NULL;
    Py alpha = NULL;
    Py beta = NULL;
    betavariate_args_t *_ARGS_  = &_riter_stat_args_(ri).beta;
    if(!(alpha = __float__(_ARGS_ ->alpha))) goto CLEAN;
    if(!(beta = __float__(_ARGS_ ->beta))) goto CLEAN;
    result = __format__("iter_beta(%zi, %R, %R)", randiter_COUNTER(ri), alpha, beta);
    CLEAN:
    Py_XDECREF(alpha);
    Py_XDECREF(beta);
    return result;
}
FUNC(Py) randiter_expo_repr(randiter *ri) {

    Py result = NULL;
    Py lambda = NULL;
    expovariate_args_t *_ARGS_  = &_riter_stat_args_(ri).expo;
    if(!(lambda = __float__(_ARGS_ ->lambda))) goto CLEAN;
    result = __format__("iter_expo(%zi, %R)", randiter_COUNTER(ri), lambda);
    CLEAN:
    Py_XDECREF(lambda);
    return result;
}
FUNC(Py) randiter_gamma_repr(randiter *ri) {

    Py result = NULL;
    Py alpha = NULL;
    Py beta = NULL;
    gammavariate_args_t *_ARGS_  = &_riter_stat_args_(ri).gamma;
    if(!(alpha = __float__(_ARGS_ ->alpha))) goto CLEAN;
    if(!(beta = __float__(_ARGS_ ->beta))) goto CLEAN;
    result = __format__("iter_gamma(%zi, %R, %R)", randiter_COUNTER(ri), alpha, beta);
    CLEAN:
    Py_XDECREF(alpha);
    Py_XDECREF(beta);
    return result;
}
FUNC(Py) randiter_lognorm_repr(randiter *ri) {

    Py result = NULL;
    Py mu = NULL;
    Py sigma = NULL;
    lognormvariate_args_t *_ARGS_  = &_riter_stat_args_(ri).lognorm;
    if(!(mu = __float__(_ARGS_ ->mu))) goto CLEAN;
    if(!(sigma = __float__(_ARGS_ ->sigma))) goto CLEAN;
    result = __format__("iter_log_normal(%zi, %R, %R)", randiter_COUNTER(ri), mu, sigma);
    CLEAN:
    Py_XDECREF(mu);
    Py_XDECREF(sigma);
    return result;
}
FUNC(Py) randiter_norm_repr(randiter *ri) {

    Py result = NULL;
    Py mu = NULL;
    Py sigma = NULL;
    normvariate_args_t *_ARGS_  = &_riter_stat_args_(ri).norm;
    if(!(mu = __float__(_ARGS_ ->mu))) goto CLEAN;
    if(!(sigma = __float__(_ARGS_ ->sigma))) goto CLEAN;
    result = __format__("iter_normal(%zi, %R, %R)", randiter_COUNTER(ri), mu, sigma);
    CLEAN:
    Py_XDECREF(mu);
    Py_XDECREF(sigma);
    return result;
}
FUNC(Py) randiter_pareto_repr(randiter *ri) {

    Py result = NULL;
    Py alpha = NULL;
    paretovariate_args_t *_ARGS_  = &_riter_stat_args_(ri).pareto;
    if(!(alpha = __float__(_ARGS_ ->alpha))) goto CLEAN;
    result = __format__("iter_pareto(%zi, %R)", randiter_COUNTER(ri), alpha);
    CLEAN:
    Py_XDECREF(alpha);
    return result;
}
FUNC(Py) randiter_triangular_repr(randiter *ri) {

    Py result = NULL;
    Py lo = NULL;
    Py hi = NULL;
    Py c = NULL;
    triangularvariate_args_t *_ARGS_  = &_riter_stat_args_(ri).triangular;
    if(!(lo = __float__(_ARGS_ ->lo))) goto CLEAN;
    if(!(hi = __float__(_ARGS_ ->hi))) goto CLEAN;
    if(!(c = __float__(_ARGS_ ->c))) goto CLEAN;
    result = __format__("iter_triangular(%zi, %R, %R, %R)", randiter_COUNTER(ri), lo, hi, c);
    CLEAN:
    Py_XDECREF(lo);
    Py_XDECREF(hi);
    Py_XDECREF(c);
    return result;
}
FUNC(Py) randiter_uniform_repr(randiter *ri) {

    Py result = NULL;
    Py a = NULL;
    Py b = NULL;
    uniformvariate_args_t *_ARGS_  = &_riter_stat_args_(ri).uniform;
    if(!(a = __float__(_ARGS_ ->a))) goto CLEAN;
    if(!(b = __float__(_ARGS_ ->b))) goto CLEAN;
    result = __format__("iter_uniform(%zi, %R, %R)", randiter_COUNTER(ri), a, b);
    CLEAN:
    Py_XDECREF(a);
    Py_XDECREF(b);
    return result;
}
FUNC(Py) randiter_vonmises_repr(randiter *ri) {

    Py result = NULL;
    Py mu = NULL;
    Py kappa = NULL;
    vonmisesvariate_args_t *_ARGS_  = &_riter_stat_args_(ri).vonmises;
    if(!(mu = __float__(_ARGS_ ->mu))) goto CLEAN;
    if(!(kappa = __float__(_ARGS_ ->kappa))) goto CLEAN;
    result = __format__("iter_vonmises(%zi, %R, %R)", randiter_COUNTER(ri), mu, kappa);
    CLEAN:
    Py_XDECREF(mu);
    Py_XDECREF(kappa);
    return result;
}
FUNC(Py) randiter_weibull_repr(randiter *ri) {

    Py result = NULL;
    Py alpha = NULL;
    Py beta = NULL;
    weibullvariate_args_t *_ARGS_  = &_riter_stat_args_(ri).weibull;
    if(!(alpha = __float__(_ARGS_ ->alpha))) goto CLEAN;
    if(!(beta = __float__(_ARGS_ ->beta))) goto CLEAN;
    result = __format__("iter_weibull(%zi, %R, %R)", randiter_COUNTER(ri), alpha, beta);
    CLEAN:
    Py_XDECREF(alpha);
    Py_XDECREF(beta);
    return result;
}
#endif