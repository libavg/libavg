export MOZILLA_ROOT=/usr/local/src/mozilla

export XPIDL=$MOZILLA_ROOT/xpcom/typelib/xpidl/xpidl
export XPIDL_FLAGS='-w -v -I '$MOZILLA_ROOT/xpcom/base

$XPIDL $XPIDL_FLAGS -m header IAVGPlayer.idl
$XPIDL $XPIDL_FLAGS -m typelib IAVGPlayer.idl
