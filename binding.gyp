{
    "targets": [
        {
            "target_name": "mt_sort",
            "sources": ["mt_sort.cc" ],
            "include_dirs" : [
 	 			"<!(node -e \"require('nan')\")"
			],
			 'conditions': [
                          [ 'OS == \'mac\'', {

                            'xcode_settings': {
                              'OTHER_CPLUSPLUSFLAGS' : ['-std=c++14']
                             }
                          }],
                        ],
        }
    ],
}