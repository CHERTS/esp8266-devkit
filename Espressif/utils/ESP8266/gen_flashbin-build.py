from distutils.core import setup
import py2exe
     
setup(
    name='gen_flashbin',
    version='0.1.0',
    license='GPLv3+',
    console=[{'script': 'gen_flashbin.py'}],
    options={'py2exe': {'includes':['binascii','re','string','sys','serial','argparse'],'optimize':2}}
)
