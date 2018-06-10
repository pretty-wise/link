import { Component, OnInit } from '@angular/core';
import { PluginService } from './plugin.service';

@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.css']
})
export class AppComponent implements OnInit {
  title = 'Web Control Panel';

  constructor(private pluginService: PluginService) {

  }
  ngOnInit() {
    this.pluginService.refresh("http://localhost:8080");
  }
}
