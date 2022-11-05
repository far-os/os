# OS ABI

## Service `0x00` - Text

### Routine `0x00` - Move cursor forward
Parameters:

  | Register | Input |
  | :- | :- |
  | `ax` | `0x0000` - Service and Routine Number |

Return values:
  
  None

### Routine `0x01` - Get cursor position
Parameters:

  | Register | Input |
  | :- | :- |
  | `ax` | `0x0001` - Service and Routine Number |

Return values:
  
  | Register | Input |
  | :- | :- |
  | `cx` | Cursor position - total character number, not split into rows | 

Info:

  ***WARNING***: upper 16 bits of ecx are trashed

### Routine `0x02` - Set cursor position
Parameters:

  | Register | Input |
  | :- | :- |
  | `ax` | `0x0002` - Service and Routine Number |
  | `cx` | Cursor position - total character number, not split into rows | 

Return values:
  
  None

### Routine `0x03` - Insert special character
Paramaters:

  | Register | Input |
  | :- | :- |
  | `ax` | `0x0003` - Service and Routine Number |
  | `dl` | What characters to insert |

Return values:

  None

Info:
  
  Can insert more than one character;

-  `dl` bit 0: `\n`
-  `dl` bit 1: `\r`
-  `dl` bit 2: `\t`

### Routine `0x04` - Write Character at Cursor
Parameters:

  | Register | Input |
  | :- | :- |
  | `ax` | `0x0004` - Service and Routine Number |
  | `bh` | Character in Codepage437 |
  | `bl` | Formatting - bits 0-2 forground colour, bit 3 foreground intensity, bit 4-6 background colour, bit 7 blink | 

Return values:
  
  None

### Routine `0x05` - Write String at Cursor
Parameters:

  | Register | Input |
  | :- | :- |
  | `ax` | `0x0005` - Service and Routine Number |
  | `esi` | Address of null-terminated string to print |
  | `bl` | Formatting - bits 0-2 forground colour, bit 3 foreground intensity, bit 4-6 background colour, bit 7 blink | 

Return values:
  
  None

### Routine `0x06` - Clear screen
Parameters:

  | Register | Input |
  | :- | :- |
  | `ax` | `0x0006` - Service and Routine Number |

Return values:
  
  None

### Routine `0x07` - Clear specific line of screen
Parameters:

  | Register | Input |
  | :- | :- |
  | `ax` | `0x0007` - Service and Routine Number |
  | `dl` | Line of screen to clear, 0-24 |

Return values:
  
  None

### Routine `0x08` - Scroll screen
Parameters:

  | Register | Input |
  | :- | :- |
  | `ax` | `0x0008` - Service and Routine Number |

Return values:
  
  None

### Routine `0x09` - Write Character at specific location
Parameters:

  | Register | Input |
  | :- | :- |
  | `ax` | `0x0009` - Service and Routine Number |
  | `bh` | Character in Codepage437 |
  | `bl` | Formatting - bits 0-2 forground colour, bit 3 foreground intensity, bit 4-6 background colour, bit 7 blink | 
  | `cx` | Location on screen to print at |

Return values:
  
  None

### Routine `0x0a` - Write String at Cursor
Parameters:

  | Register | Input |
  | :- | :- |
  | `ax` | `0x000a` - Service and Routine Number |
  | `esi` | Address of null-terminated string to print |
  | `bl` | Formatting - bits 0-2 forground colour, bit 3 foreground intensity, bit 4-6 background colour, bit 7 blink | 
  | `cx` | Location on screen to print at |

Return values:
  
  None
