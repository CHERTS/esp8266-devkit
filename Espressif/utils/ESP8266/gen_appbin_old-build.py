from distutils.core import setup
import py2exe
     
setup(
    name='gen_appbin_old',
    version='0.1.0',
    license='GPLv3+',
    console=[{'script': 'gen_appbin_old.py'}],
    options={'py2exe': {'includes':['binascii','re','os','re','string','sys','serial','argparse','json','hashlib','inspect','base64','zlib','shlex','time','struct'],'optimize':2}}
)
